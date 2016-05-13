#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <linux/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
} 

int main(int argc, char *argv[])
{
int socket_fd,port_no;
struct sockaddr_in client_addr;

char *dst_addr="255.255.255.255";
char *src_addr="192.168.1.4";
char arr[4096];
memset(arr,0,4096);
struct iphdr *head=(struct iphdr *)arr;
char *data; 
data= arr + sizeof(struct iphdr);
printf("contents of data before strcpy are %s\n",data);
strcpy(data, "broadcast message");
printf("after strcpy- data is %s\n",data);
if(argc<0){
error(" hostname error\n");
}

bzero((char *) &client_addr, sizeof(client_addr));
port_no = atoi(argv[2]);
printf("port number is %d",port_no);
socket_fd=socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
if (socket_fd < 0) {
	error("Error opening socket");
}
else
{
printf("opened socket");}

client_addr.sin_family = AF_INET;
client_addr.sin_addr.s_addr=inet_addr(dst_addr);
client_addr.sin_port = htons(port_no);
	
int broadcast=1;
if (setsockopt(socket_fd,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast))==-1) {
    error("Error in setting the socket option for broadcast");
}
else
{
printf("set option broadcast\n");
}

int hdr=1;	
if (setsockopt(socket_fd,IPPROTO_IP,IP_HDRINCL,&hdr,sizeof(hdr))==-1) {
    error("Error setting the socket option for header");
}
else
{
printf("set option socket\n");
}	

head->ihl = 5;
head->version = 4;
head->tot_len = sizeof(struct iphdr);
//head.tos = 
//head->id = 0;
head->ttl = 64;
head->protocol = IPPROTO_IP;
printf("length of data is %d\n",(int)(strlen(data)));
head->tot_len=sizeof(struct iphdr)+strlen(data);
printf("total length of head is %d\n",head->tot_len);
head->saddr=inet_addr(src_addr);
head->daddr=client_addr.sin_addr.s_addr;

//printf("Please enter the message: ");
//bzero(arr,256);
//fgets(arr,255,stdin);

printf("contents of data are %s\n",data);
printf("after ip header\n");
for(;;){
int num_bytes;
num_bytes=sendto(socket_fd,head,head->tot_len, 0, (struct sockaddr *) &client_addr, sizeof(client_addr)) ;
if(num_bytes <0)
{            
 error("Error in sending the packet\n");
}
else{
printf("sent successfully\n");}
printf("sent %d bytes to %s\n",num_bytes,inet_ntoa(client_addr.sin_addr));

sleep(1);		 
}
close(socket_fd);	
return 0;
}
