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
#include "gps.h"

void error(const char *msg)	//to display error messages
{
	perror(msg);
	exit(0);
}

struct Blob1 {

	// Part I Content, sent at all times
	uint8_t   msgCnt;      // Message count                      -- 1 byte
	uint32_t   id;           // A temp ID field                    -- 4 bytes
	uint16_t  secMark;      // the current second                 -- 2 bytes

							// pos      PositionLocal3D,
	float lat;    // lat         Latitude,              -- 4 bytes
	float longi;   // long        Longitude,             -- 4 bytes          // note: pick a different name to avoid collisions
	uint16_t elev;     // elev        Elevation,             -- 2 bytes
	uint32_t accuracy;  // accuracy    PositionalAccuracy,    -- 4 bytes

						//  motion   Motion,
	uint16_t speed;  // speed       Speed and Transmission -- 2 bytes
	uint16_t heading;  // heading     Heading,               -- 2 bytes
	uint8_t angle;  // steering    Angle,                 -- 1 byte

					// accelSet    AccelerationSet4Way,  -- accel set (four way) 7 bytes
	uint16_t accelLong;
	uint16_t accelLatSet;
	uint16_t accelVert;
	uint8_t accelYaw;

	// -- control  Control,
	uint16_t brakes;  // brakes      BrakeSystemStatus,     -- 2 bytes

					  // -- basic    VehicleBasic,   VehicleSize,          -- 6 bytes
	uint8_t size1;  // size (width)
	uint8_t size2;  // size (width)
	uint8_t size3;  // size (width)
	uint8_t size4;  // size (size)
	uint8_t size5;  // size (size)
	uint8_t size6;  // size (size)

};

enum DSRCmsgID {	//enum variable for msgID
	DSRCmsgID_reserved = 0,
	DSRCmsgID_alaCarteMessage = 1,
	DSRCmsgID_basicSafetyMessage = 2,
};

struct BasicSafetyMessage {	//structure of BSM
	enum DSRCmsgID msgID;  		 // A 1 byte instance
	struct Blob1 blob1;             // A 38 byte instance
									// struct VehicleSafetyExtension *safetyExt;    //optional
									// struct VehicleStatus *status;                //optional
									//asn_struct_ctx_t _asn_ctx;                    //optional
};

int main(int argc, char *argv[])
{
	int socket_fd, port_no, i,j;
	char arr[4096];		//buffer to store payload
	memset(arr, 0, 4096);

	struct BasicSafetyMessage *BSM_head = (struct BasicSafetyMessage *)arr;		//pointer to BSM
	//char *data;	//contents of packet
	//data = arr + sizeof(struct BasicSafetyMessage);
	//strcpy(data, "broadcast BSM");
	printf("size of BSM is %lu\n", sizeof(struct BasicSafetyMessage));
	struct gps_data_t gpsdata;
	j = gps_open("localhost", "3333", &gpsdata); // can be any port number
	printf("open status is %d\n", j);
	(void)gps_stream(&gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
	printf("overall status is %d\n", gpsdata.status);
	
	if (argc<0) {
		error(" hostname error\n");
	}

	port_no = atoi(argv[1]);
	printf("port number is %d\n", port_no);

	socket_fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));	//ethernet header automatically constructed
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
	printf("/n%d", ifr.ifr_name);
	if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) == -1) {
		error("error in ioctl block\n");
	}
	int ifindex = ifr.ifr_ifindex;	//store interface number in the variable 'ifindex'


	const unsigned char ether_broadcast_addr[] = { 0xff,0xff,0xff,0xff,0xff,0xff };	//destination address- broadcast address in this case

	struct sockaddr_ll addr = { 0 };
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = ifindex;
	addr.sll_halen = ETHER_ADDR_LEN;
	addr.sll_protocol = htons(ETH_P_ALL);
	memcpy(addr.sll_addr, ether_broadcast_addr, ETHER_ADDR_LEN);


	int broadcast = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {	//setting the 'broadcast' option on socket
		error("Error in setting the socket option for broadcast");
	}
	else
	{
		printf("set option broadcast\n");
	}

	//structure of BSM
	BSM_head->msgID = DSRCmsgID_basicSafetyMessage;
	BSM_head->blob1.msgCnt = 0x01;
	BSM_head->blob1.id = 0x20;
	BSM_head->blob1.secMark =0xea60;
	
	BSM_head->blob1.heading = 0x3840;
	BSM_head->blob1.angle = 0x0;
	BSM_head->blob1.accuracy = 00000000;

	int tot_len = sizeof(struct BasicSafetyMessage);
	for (;;) //to keep broadcasting continuously
	{
		int num_bytes;
		if (gps_read(&gpsdata) == -1) {
				printf("error inside read\n");
		}
		else {
			printf("checking for > 0\n");
			// We have data
			if (gpsdata.status > 0) {
			//	BSM_head->blob1.lat =htonl(gpsdata.fix.latitude);
				BSM_head->blob1.lat=43.004605;
				BSM_head->blob1.longi =htonl(gpsdata.fix.longitude);
				BSM_head->blob1.elev = htonl(gpsdata.fix.altitude);
				BSM_head->blob1.speed = htonl(gpsdata.fix.speed);
				printf("initial latitude is %lf\n", gpsdata.fix.latitude);
			}
		}

		num_bytes = sendto(socket_fd, BSM_head, tot_len, 0, (struct sockaddr*)&addr, sizeof(addr));//sendto() to broadcast the BSM packet

		if (num_bytes <0) {
			error("Error in sending the packet\n");
		}
		else
		{
			printf("sent %d bytes\n", num_bytes);
		}
		sleep(0.3);
	}
	close(socket_fd);
	(void)gps_stream(&gpsdata, WATCH_DISABLE, NULL);
	(void)gps_close(&gpsdata);
	return 0;
}


