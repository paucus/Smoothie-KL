/*
 * TCPFacade.h
 *
 *  Created on: Oct 28, 2015
 *      Author: eai
 */

#ifndef TCPFACADE_H_
#define TCPFACADE_H_

#include "uip.h"
using namespace std;
#include <string>
#include <string.h>
#include <inttypes.h>
#include <functional>

#define MAX_TCPFACADE_CONNS 2

typedef enum tcpfacade_state {
	NN_PRE_CONNECTION, NN_CONNECTED, NN_DATA_READY_TO_SEND, NN_DATA_WAITING_ACK
} tcpfacade_state_t;

// TCPP_OK = No error
// TCPP_CANCEL = User requested cancel
// TCPP_ERROR = An exception occurred and connection must be finished
// TCPP_RESP = No error, but a response needs to be sent
typedef enum tcpprogress_resp {
	TCPP_OK, TCPP_CANCEL, TCPP_ERROR, TCPP_RESP
} tcpprogress_resp_t;

typedef enum tcp_connect_resp {
	TCPC_OK, TCPC_DNS_RETRY, TCPC_ERROR, TCPC_TOO_MANY_CONNS
} tcp_connect_resp_t;

// List of error for error_callback_t
#define TCP_CLLBCK_ERR_INVALID_STATE 1
#define TCP_CLLBCK_ERR_HOST_UNKNOWN  2
#define TCP_CLLBCK_ERR_DNS_TIMEOUT   3
// TCP_CLLBCK_ERR_APPLAYER_BASE MUST be the last value.
// Any application layer error must start from TCP_CLLBCK_ERR_APPLAYER_BASE on, to ensure no
// overlapping occurs with TCP errors.
#define TCP_CLLBCK_ERR_APPLAYER_BASE 4
#define translate_tcp_err(e) ( \
			(e == TCP_CLLBCK_ERR_INVALID_STATE)? "Invalid State": \
			(e == TCP_CLLBCK_ERR_HOST_UNKNOWN)? "Host unknown": \
			(e == TCP_CLLBCK_ERR_DNS_TIMEOUT)? "DNS Timeout": \
			"Unknown error"\
		)


typedef std::function<tcpprogress_resp_t(const void* data, uint16_t len)> data_recv_callback_t;
typedef std::function<void(char** data, uint16_t* len)> answer_callback_t;
typedef std::function<void()> close_callback_t;
typedef std::function<void(int err_num)> error_callback_t;

typedef struct callback_set {
	data_recv_callback_t data_recv_callback;
	data_recv_callback_t data_ack_callback;
	answer_callback_t answer_callback;
	close_callback_t close_callback;
	error_callback_t error_callback;
	callback_set(data_recv_callback_t data_recv_callback,
			answer_callback_t answer_callback,
			close_callback_t close_callback,
			error_callback_t error_callback) :
				data_recv_callback(data_recv_callback),
				answer_callback(answer_callback),
				close_callback(close_callback),
				error_callback(error_callback) {};
} callback_set_t;

class TCPFacade {
public:
	TCPFacade(const void* data, uint16_t len, callback_set_t callback_set);
	virtual ~TCPFacade();

	// This set of functions are very basic. They connect to the given address and port, and sends
	// the given data. After that it waits for the response and calls the callback function for
	// every chunk of data received. It is responsibility of the caller to provide a callback
	// function that handles data splitting correctly.
	// When the connection is closed, the close_callback function is called.
	// The function doesn't get blocked for the whole download process. It returns control after
	// the connection is established. The return value only reflects the error status up to
	// that moment. If something fails during the download process the download operation will be
	// cancelled.
	// If the DNS name is not known, it will ask the caller to retry the call once the DNS response
	// is received.
	static tcp_connect_resp_t send_data_to(const char* dest, uint16_t port, const void* data, uint16_t len, callback_set_t callback_set);
	static tcp_connect_resp_t send_data_to(const char* dest, uint16_t port, const char* data, callback_set_t callback_set);
	static tcp_connect_resp_t send_data_to(const char* dest, uint16_t port, string& data, callback_set_t callback_set);

	// For internal usage
	static void periodic_call();
	static bool is_tcpfacade_conn(struct uip_conn* conn);

private:
	void process_input();
	void send_data();
	// "Data" stores the current message to be sent. This message could need to be split in many
	// segments to be transmitted to the other end.
	char* data;
	// "Len" is the total length of data.
	uint16_t len;
	// "Sent" stores the number of bytes sent and confirmed to have been received by the other end.
	uint16_t sent;
	// "Len_to_ack" stores the number of bytes sent but not yet confirmed to have been received.
	uint16_t len_to_ack;
	volatile tcpfacade_state_t state;
	// This function will be called for every chunk of data received. It returns a boolean saying
	// if the process must continue or it must be cancelled.
	callback_set_t callback_set;

	static void add_conn(struct uip_conn* conn);
	static void remove_conn(struct uip_conn* conn);
	static volatile uint8_t available_conns;
	static struct uip_conn* conns[MAX_TCPFACADE_CONNS];
};

#endif /* TCPFACADE_H_ */
