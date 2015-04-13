
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UDP_PORT 4013
#define BUFLEN 1024
 
struct sockaddr_in multicast_addr;
struct sockaddr_in udp_addr;
struct sockaddr_in udp_client_addr;
struct ip_mreq group;
int mc_sock; 	// the multicast socket file descriptor
int udp_sock; 	// the udp socket file descriptor
int datalen;
int recvlen;
char mcbuf[BUFLEN];
char udpbuf[BUFLEN];
char *multicast_ip;
int multicast_port;
 
int main(int argc, char *argv[])
{
	int reuse = 1;
	int server_length = sizeof(struct sockaddr_in);
	int client_length = sizeof(struct sockaddr_in);

	/* Check for command line parameters */
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s Multicast-IP Multicast-Port\n", argv[0]);
		exit(1);
	}

	multicast_ip = argv[1];      			/* arg 1: multicast ip address */
	multicast_port = atoi(argv[2]);   /* arg 2: multicast port number */
	
	/* Create a socket on which to receive multicast packets */
	if ((mc_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Opening multicast socket error");
		exit(1);
	}

	/* Create a socket for the UDP server */
	if ((udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("Opening UDP socket error");
		exit(1);
	}

	 
	/* Set REUSE port to ON to allow multiple binds per host */
	if(setsockopt(mc_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(mc_sock);
		exit(1);
	}
	 
	/* Construct a multicast address structure */
	memset(&multicast_addr, 0, sizeof(multicast_addr));
	multicast_addr.sin_family = AF_INET;
	multicast_addr.sin_port = htons(multicast_port);
	multicast_addr.sin_addr.s_addr = INADDR_ANY;

	/* Construct a UDP address structure */
	memset(&udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(UDP_PORT);
	udp_addr.sin_addr.s_addr = INADDR_ANY;
	
	/* Bind multicast address to socket */
	if(bind(mc_sock, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)))
	{
		perror("Binding multicast socket error");
		close(mc_sock);
		exit(1);
	}

	/* Bind UDP socket to port */
	if (bind(udp_sock, (struct sockaddr*)&udp_addr, sizeof(udp_addr)))
	{
		perror("Binding UDP socket error");
		close(udp_sock);
		exit(1);
	}

  	/* construct an IGMP join request structure */
	group.imr_multiaddr.s_addr = inet_addr(multicast_ip);
	group.imr_interface.s_addr = INADDR_ANY;
		
	/* send an ADD MEMBERSHIP message via setsockopt */
	if(setsockopt(mc_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(mc_sock);
		exit(1);
	}

	printf("Waiting for UDP connection....\n");
	
	/* Read a message from the UDP client so that we can get its IP address */
	if ((recvlen = recvfrom(udp_sock, 
				udpbuf, 
				BUFLEN, 
				0, 
				(struct sockaddr *) &udp_client_addr, 
				&client_length)) == -1)
  	{
		perror("UDP packet receive failed");
		exit(1);
	}
	else
	{
		printf("Connection to %s port %d successful\n", 
			inet_ntoa(udp_client_addr.sin_addr), 
			ntohs(udp_client_addr.sin_port));
	}
	
	printf("Listening to multicast address %s port %d\n", multicast_ip, multicast_port);
	
	while(1)
	{
		/* Read from the multicast socket. */
		datalen = sizeof(mcbuf);
		if(read(mc_sock, mcbuf, datalen) < 0)
		{
			perror("Reading datagram message error");
			close(mc_sock);
			close(udp_sock);
			exit(1);
		}

		/* Send the multicast message to the UDP client */
		if ((sendto(udp_sock, 
			mcbuf, 
			(int)strlen(mcbuf) + 1, 
			0, 
			(struct sockaddr *) &udp_client_addr, 
			server_length)) == -1)
		{
			perror("Data tranmission to UDP client failed");
			close(mc_sock);
			close(udp_sock);
			exit(1);
		}
	}
	return 0;
}
