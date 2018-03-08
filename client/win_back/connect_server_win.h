#ifndef _CONNECT_SERVER_H_
#define _CONNECT_SERVER_H_

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>	 
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>




#define SVR_TOKEN  "Hello client"
#define CLT_TOKEN  "Hello server"

extern int udp_socket;

int connect_server(char *, unsigned short );
int udp_broadcast(struct sockaddr_in* , char *, unsigned short );
//SIGPIPE�źŻص�����
void sig_callback(int sig);
//��TCP���ӷ�����,�ɹ�����������������ʧ�ܷ���-1
int tcp_connect(struct sockaddr_in* pSvr_addr);





#endif

