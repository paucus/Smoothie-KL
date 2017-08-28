/*
 * TCPFacade.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: eai
 */

#include "TCPFacade.h"
#include "uip.h"
#include "SerialMessage.h"
#include "StreamOutput.h"

#define DEBUG_PRINTF printf

volatile uint8_t TCPFacade::available_conns = MAX_TCPFACADE_CONNS;
struct uip_conn* TCPFacade::conns[MAX_TCPFACADE_CONNS];	// C++ standard specifies it's zero initialized

#define TCP_SEGMENT_SIZE UIP_APPDATA_SIZE

TCPFacade::TCPFacade(const void* data, uint16_t len, callback_set_t callback_set) : len(len), sent(0), len_to_ack(0), state(NN_PRE_CONNECTION), callback_set(callback_set) {
	this->data = new char[len];
	memcpy(this->data, data, len);
}

TCPFacade::~TCPFacade() {
	if (this->data)
		delete [] this->data;
}

void TCPFacade::periodic_call() {
	TCPFacade *instance = reinterpret_cast<TCPFacade *>(uip_conn->appstate);
	if (!instance) {
		// Something is wrong!
		uip_abort();
		return;
	}

	if (uip_closed() || uip_aborted() || uip_timedout()) {
		if (instance && uip_newdata()) {
			// Data received.
			instance->process_input();
		}
		if(instance != NULL) {
			instance->callback_set.close_callback();
			remove_conn(uip_conn);	// Remove it from the list of connections
			delete instance;
			uip_conn->appstate = NULL;
		}
		return;
	}

	if (uip_connected()) {
		// New connection established.
//		instance->state = NN_CONNECTED;
		// We can omit NN_CONNECTED state because we know we have something to send
		instance->state = NN_DATA_READY_TO_SEND;
	} else if (instance->state == NN_PRE_CONNECTION) {
		// Not connected. Nothing to do yet.
		return;
	}

	if (uip_newdata()) {
		// Data received.
		instance->process_input();
	}

	if (uip_poll()) {
		// The application is being asked for new data. If there's some, send it.
		if (instance->state != NN_DATA_READY_TO_SEND) {
			// No new data. Exit.
			return;
		}
		if (instance->len_to_ack > 0) {
			// Still waiting for ack.
			return;
		}
		instance->send_data();
	} else if (uip_rexmit()) {
		// Retransmission requested. Repeat the same data.
		if (instance->state != NN_DATA_WAITING_ACK) {
			// Assertion error!! No new data should have been queued!!
			if (instance->callback_set.error_callback) instance->callback_set.error_callback(TCP_CLLBCK_ERR_INVALID_STATE);
			return;
		}
		instance->send_data();
	} else if (uip_acked()) {
		// Mark data as sent
		instance->sent += instance->len_to_ack;
		instance->len_to_ack = 0;
		// If there are more fragments to send, do it.
		if (instance->sent < instance->len) {
			instance->send_data();
			// Don't change the state to connected. We have more data to transmit.
			return;
		}

		tcpprogress_resp_t resp;
		if (instance->callback_set.data_ack_callback) {
			resp = instance->callback_set.data_ack_callback(instance->data, instance->sent);
		} else {
			resp = TCPP_OK;
		}

		// Task done. Don't close the connection because we could receive an answer. Also the
		// sender might want to send a response.
		if (instance->state == NN_DATA_WAITING_ACK) {
			// Reset counters
			instance->len_to_ack = 0;
			instance->sent = 0;
			instance->state = NN_CONNECTED;
		}

		if (resp == TCPP_CANCEL || resp ==  TCPP_ERROR) {
			// Error. Close connection.
			uip_close();
		} else if (resp == TCPP_RESP) {
			// More data must be sent
			if (instance->data) delete [] instance->data; // Delete previous buffer so that a new one can be allocated
			instance->data = nullptr; // Mark it as null in case the answer callback does not reserve space
			instance->len = 0;
			instance->len_to_ack = 0;
			instance->sent = 0;
			if (instance->callback_set.answer_callback) instance->callback_set.answer_callback(&instance->data, &instance->len);
			if (instance->data) {
				instance->state = NN_DATA_READY_TO_SEND; // Mark status as ready to send data
			}
		}
	}
}

static bool match_str_ip_addr(const char* dest, uip_ipaddr_t* addr) {
	int a1, a2, a3, a4;
	if (sscanf(dest, "%d.%d.%d.%d", &a1, &a2, &a3, &a4) == 4) {
		// IP address
		if (a1 <= 0 || a1 > 255) return false;
		if (a2 <= 0 || a2 > 255) return false;
		if (a3 <= 0 || a3 > 255) return false;
		if (a4 <= 0 || a4 > 255) return false;
		uip_ipaddr(addr, a1, a2, a3, a4);
		return true;
	} else {
		return false;
	}
}

static void to_uip_addr_t(const u16_t* dns_ip, uip_ipaddr_t* addr) {
	memcpy(addr, dns_ip, sizeof(uip_ipaddr_t));
}

tcp_connect_resp_t TCPFacade::send_data_to(const char* dest, uint16_t port, const void* data, uint16_t len, callback_set_t callback_set) {
	if (TCPFacade::available_conns == 0) {
		// Not enough available connections
		DEBUG_PRINTF("Not enough available connections\n");
		return TCPC_TOO_MANY_CONNS;
	}
	struct uip_conn *conn;
	uip_ipaddr_t addr;
	if (!match_str_ip_addr(dest, &addr)) {
		// Destination address is not an ip address. We must resolve it first.
		// First check in the internal cache.
		u16_t* ip = resolv_lookup(dest);
		if (!ip || (!ip[0] && !ip[1])) { // ip = NULL or 0.0.0.0 (Check index 0 & 1 because they are 16 bits long each)
			// We don't know it, we must query the DNS server.
			resolv_query(dest);
			return TCPC_DNS_RETRY; // So far we can't say connection will work or fail. Answer with an "OK".
		}

		// That name was already cached.
		to_uip_addr_t(ip, &addr);
	}
	// Destination IP address known. Proceed now.

	conn = uip_connect(&addr, HTONS(port));
	if (conn == NULL) {
		DEBUG_PRINTF("Failed to connect\n");
		return TCPC_ERROR;
	} else {
		add_conn(conn);
		TCPFacade* nn = new TCPFacade(data, len, callback_set);
		conn->appstate = nn;
		return TCPC_OK;
	}
}
tcp_connect_resp_t TCPFacade::send_data_to(const char* dest, uint16_t port, const char* str, callback_set_t callback_set) {
	// Don't consider the '\0' at the end
	return TCPFacade::send_data_to(dest, port, str, strlen(str), callback_set);
}
tcp_connect_resp_t TCPFacade::send_data_to(const char* dest, uint16_t port, string& str, callback_set_t callback_set) {
	// Don't consider the '\0' at the end
	return TCPFacade::send_data_to(dest, port, str.c_str(), str.length(), callback_set);
}

void TCPFacade::process_input(){
    uint16_t len;
    char *dataptr;
    len = uip_datalen();
    dataptr = (char *)uip_appdata;

    tcpprogress_resp_t resp = this->callback_set.data_recv_callback(dataptr, len);
    if (resp == TCPP_RESP) {
    	// Queue a response
    	if (this->data) delete [] this->data; // Delete previous buffer so that a new one can be allocated
    	this->data = nullptr; // Mark it as null in case the answer callback does not reserve space
    	this->len = 0;
    	this->len_to_ack = 0;
    	this->sent = 0;
    	if (callback_set.answer_callback) this->callback_set.answer_callback(&this->data, &this->len);
    	if (this->data) {
			this->state = NN_DATA_READY_TO_SEND; // Mark status as ready to send data
    	}

	} else if (resp != TCPP_OK) {
		// Cancel current download
		uip_close();
	}
}

void TCPFacade::send_data() {
	if (!uip_closed()) {
		this->len_to_ack = min(TCP_SEGMENT_SIZE, this->len - this->sent);
		uip_send(&(this->data[this->sent]), this->len_to_ack);
		this->state = NN_DATA_WAITING_ACK;
	}
}

bool TCPFacade::is_tcpfacade_conn(struct uip_conn* conn) {
	for(int i = 0; i < MAX_TCPFACADE_CONNS; i++) {
		if (TCPFacade::conns[i] == conn){
			return true;
		}
	}
	return false;
}

void TCPFacade::add_conn(struct uip_conn* conn) {
	for(int i = 0; i < MAX_TCPFACADE_CONNS; i++) {
		if (!TCPFacade::conns[i]){
			TCPFacade::conns[i] = conn;
			TCPFacade::available_conns--;
			return;
		}
	}
}
void TCPFacade::remove_conn(struct uip_conn* conn) {
	for(int i = 0; i < MAX_TCPFACADE_CONNS; i++) {
		if (TCPFacade::conns[i] == conn){
			TCPFacade::conns[i] = NULL;
			TCPFacade::available_conns++;
			return;
		}
	}
}
