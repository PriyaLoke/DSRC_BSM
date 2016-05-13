#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <net/if.h>
#include <stdint.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h> 
#include <arpa/inet.h>
#include <linux/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)	//to display error messages
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
//	int broadcast=1;
	int socket_fd;
	struct sockaddr_ll addr;
	socklen_t sll_len = (socklen_t)sizeof(addr);
	//struct sockaddr saddr;

	unsigned char *buffer = (unsigned char *)malloc(65536);
	memset((void*)&addr, 0, sizeof(addr));

	socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));	//ethernet header automatically constructed
	if (socket_fd < 0) {
		error("Error opening socket");
	}
	else
	{
		printf("opened socket\n");
	}
	
	//setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST,&broadcast, sizeof broadcast);
	//saddr.sin_family = AF_INET;
	//saddr.sin_addr.s_addr = INADDR_ANY;
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(ETH_P_ALL);
	//sock.sll_pkttype = PACKET_BROADCAST;
	/*
	struct ifreq ifr;
	size_t if_name_len = strlen("wlan0");
	if (if_name_len<sizeof(ifr.ifr_name)) {
		memcpy(ifr.ifr_name, "wlan0", if_name_len);
		ifr.ifr_name[if_name_len] = 0;
	}
	else {
		error("interface name is too long\n");
	}

	if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) == -1) {
		error("error in ioctl block\n");
	}
		//store interface number in the variable 'ifindex'
	sock.sll_ifindex = ifr.ifr_ifindex;

	if (-1 == bind(socket_fd, (struct sockaddr *) &sock, sizeof(sock))) {
		perror("packet_receive_renew::bind");
		close(socket_fd);
		return -3;
	}

	if (-1 == setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast))) {
		perror("packet_receive_renew::setsockopt");
		close(socket_fd);
		return -1;
*/


	while (1)
	{
		//saddr_size = sizeof(saddr);
		//Receive a packet
		int num_bytes;
		num_bytes = recvfrom(socket_fd, buffer, 65536, 0, (struct sockaddr *)&addr, &sll_len);//sendto() to broadcast the BSM packet

		if (num_bytes <0) {
			error("Error in receiving the packet\n");
		}
		else
		{
			printf("received %d bytes\n", num_bytes);
			int interface_index = addr.sll_ifindex;// Interface index, as used in sending 
			unsigned short packet_type = addr.sll_pkttype; // 0=unicast to me, 1=multicast, 2=broadcast, etc 
		//	printf("packet type is %hu\n",packet_type);
			printf("interface number is %d\n",interface_index);
			/* More fields for source address, protocol number, etc */
		}

	}
	
		

	printf("Finished");
	close(socket_fd);
	return 0;
}



