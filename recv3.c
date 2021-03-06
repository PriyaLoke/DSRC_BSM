#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
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
#include "gps.h"

void error(const char *msg)	//to display error messages
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int socket_fd, i,j,s;
	unsigned long c;
	float degrees;
	char arr[4096];		//buffer to store payload
	memset(arr, 0, 4096);
	char temp[4];
	uint32_t num;
	
	socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));	
	if (socket_fd < 0) {
		error("Error opening socket");
	}
	else
	{
		printf("opened socket\n");
	}

	//to determine interface number of the interface to be used
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
	int ifindex = ifr.ifr_ifindex;	//store interface number in the variable 'ifindex'

	// Set interface to promiscuous mode
	strncpy(ifr.ifr_name, "wlan0", if_name_len);
	ioctl(socket_fd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(socket_fd, SIOCSIFFLAGS, &ifr);

	
	/* Bind to device */
	if (setsockopt(socket_fd, SOL_SOCKET, SO_BINDTODEVICE, "wlan0", if_name_len) == -1)	{
		error("Error in binding to socket\n");
		close(socket_fd);
	}

	for (;;) //to keep receiving continuously
	{
		int num_bytes;
		memset(arr,0,4096);
		num_bytes = recvfrom(socket_fd, arr, 69, 0, NULL, NULL);//receive the BSM packet
		
		if (num_bytes <0) {
			error("Error in receiving the packet\n");
		}
		else
		{
			printf("received %d bytes\n", num_bytes);
		}
		printf("The contents of the buffer are:\n");
		for(s=0;s<num_bytes;s=s+2){
			printf("%02x %02x\n",arr[s+1],arr[s]);
		}
		
		printf("before memcpy");
		printf("19- %02\nx",arr[19]);
		printf("18- %02x\n",arr[18]);
		printf("17= %02x\n",arr[17]);
		printf("16-%02x\n",arr[16]);
		sprintf(temp,"%x%x%x%x",arr[19],arr[18],arr[17],arr[16]);
		printf("the hex string is %s",temp);
	/*	memcpy(subbuff,arr+19,sizeof(char));
		memcpy(subbuff+2,arr+18,sizeof(char));
		memcpy(subbuff+4,arr+17,sizeof(char));
		memcpy(subbuff+6,arr+16,sizeof(char));
		subbuff[8] = '\0';
		
		printf("sub buffer is %s",subbuff);
		sscanf(subbuff, "%x", &num);  // assuming you checked input
		degrees = *((float*)&num);
		printf("the hexadecimal 0x%08x becomes %.6f as a float\n", num, degrees);

		printf("The latitude is %.6f\n",degrees);*/
		sleep(0.3);
	}
	
	close(socket_fd);
	return 0;
}


