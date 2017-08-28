#ifndef NETWORKPUBLICACCESS_H
#define NETWORKPUBLICACCESS_H

#define network_checksum        CHECKSUM("network")
#define get_ip_checksum         CHECKSUM("getip")
#define get_ipconfig_checksum   CHECKSUM("getipconfig")
// BEGIN MODIF net_notif
#define get_gw_checksum         CHECKSUM("getgw")
#define get_ipmask_checksum     CHECKSUM("getipmask")
#define get_dnsaddr_checksum    CHECKSUM("getdnsaddr")
#define get_macaddr_checksum    CHECKSUM("getmacaddr")
// END MODIF net_notif

#endif
