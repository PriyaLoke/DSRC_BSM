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
	int socket_fd, i,j,s,num_bytes;
	uint8_t arr[60]; //buffer to store payload
	char temp[5];
	char temp1[3];
	uint32_t num,times,seq;		
	float degrees1,degrees2;

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
	
	FILE *fpr;
	fpr = fopen("logger_recv.txt","a");
	fprintf(fpr,"Timestamp \t SeqNum \t Latitude \t Longitude \n");

	for (;;) //to keep receiving continuously
	{
		memset(arr, 0, 60);
		num_bytes = recvfrom(socket_fd, arr, 56, 0, NULL, NULL);//receive the BSM packet
		
		if (num_bytes <0) {
			error("Error in receiving the packet\n");
		}
		else
		{
			printf("\n\nNew packet - received %d bytes\n", num_bytes);
		}
/*
		printf("The contents of the buffer are:\n");
		for(s=0;s<num_bytes;s=s+2){
			printf("%02x %02x\n",arr[s+1],arr[s]);
		}

		printf("19- %02x\n",arr[19]);
		printf("18- %02x\n",arr[18]);
		printf("17- %02x\n",arr[17]);
		printf("16- %02x\n",arr[16]);
*/		
		snprintf(temp,sizeof(temp),"%02x%02x%02x%02x",arr[19],arr[18],arr[17],arr[16]);	
		printf("The latitude is %s\n",temp);
		
		sscanf(temp,"%x",&num);
		degrees1=*((float*)&num);
		printf("the hexadecimal %10x becomes %.6f as a float\n",temp,degrees1);
/*		
		printf("23- %02x\n",arr[23]);
		printf("22- %02x\n",arr[22]);
		printf("21- %02x\n",arr[21]);
		printf("20- %02x\n",arr[20]);
*/		
		snprintf(temp,sizeof(temp),"%.2x%.2x%.2x%.2x",arr[23],arr[22],arr[21],arr[20]);
		printf("The longitude is %s\n",temp);

		sscanf(temp,"%x",&num);
		degrees2=*((float*)&num);
		printf("the hexadecimal %10x becomes %.6f as a float\n",temp,degrees2);
/*		
		printf("13- %02x\n",arr[13]);
		printf("12- %02x\n",arr[12]);	
*/		
		snprintf(temp,sizeof(temp),"%.2x%.2x",arr[13],arr[12]);
		printf("The timestamp is %s\n",temp);
		sscanf(temp,"%x",&num);
		printf("the hexadecimal %x becomes %u as a number\n",temp,num);
		times = num;

		//printf("4 - %02x\n",arr[4]);
		sprintf(temp,"%.2x",arr[4]);
		printf("The sequence number is %s\n",temp);
		sscanf(temp,"%x",&num);
		printf("the hexadecimal %10x becomes %u as an int\n",temp,num);
		seq = num;
		
		fprintf(fpr,"%u",times);
		fprintf(fpr,"\t\t");
		fprintf(fpr,"%u",seq);
		fprintf(fpr,"\t");
		fprintf(fpr,"%f",degrees1);
		fprintf(fpr,"\t");
		fprintf(fpr,"%f",degrees2);
		fprintf(fpr,"\n");
		//sleep(0.3);
	}
	
	close(socket_fd);
	return 0;
}


