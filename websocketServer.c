#define DEBUG					1
#define DEFEULT_SERVER_PORT		8888
#define MAX_RX_LEN				1024
#define MAX_TX_LEN				1024
#define MAX_WEB_SOCKET_KEY_LEN	256
#define IMAGE_NAME				"websocketPic.jpg"
#define ACK_KEY					"amiok"
#define TRANS_INTERVAL			100		// ms

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include "encoder.h"

unsigned char rxBuf[MAX_RX_LEN];
unsigned char txBuf[MAX_TX_LEN];

void usage(char *name) {
	printf("Usage: %s [Port Number(Default:%d)]\n", name, DEFEULT_SERVER_PORT);
}

int recvAck(int clientFd) {
	unsigned char finFlag, maskFlag, mask[4];
	unsigned long payloadLen;
	unsigned char *payloadData;
	int i;

	memset(rxBuf, 0, MAX_RX_LEN);
    read(clientFd, rxBuf, MAX_RX_LEN);                        

	finFlag=((rxBuf[0] & 0x80) == 0x80);
	if( !finFlag ) {
		printf("Do not process non-final fragment\n");
		return -1;
	}

	maskFlag=((rxBuf[1] & 0x80) == 0x80);
	if( maskFlag ) {
		if(DEBUG)
			printf("Receive mask data\n");
	} else {
		printf("Do not process non-mask data.\n");
		return -1;
	}

	payloadLen=(rxBuf[1] & 0x7F);
	if(payloadLen > 125 ) {
		printf("do not process payload which is over than 125.\n");
		return -1;
	}

	if(payloadLen < 126) {
		memcpy(mask, rxBuf+2, 4);
		payloadData=(char *)malloc(payloadLen);
		memset(payloadData, 0, payloadLen);
		memcpy(payloadData, rxBuf+6, payloadLen);
	}

	for( i=0 ; i<payloadLen ; i++) {
		payloadData[i] = (unsigned char)(payloadData[i] ^ mask[i % 4]);
	}
	if(DEBUG)
		printf("(%ld):%s\n", payloadLen, payloadData);
	if(strcmp(payloadData, ACK_KEY) != 0)
	{
		printf("ack error\n");
		free(payloadData);
		return -1;
	}
	free(payloadData);
	return 0;
}

int fromWebSocketHeader(unsigned char *sendBuffer, unsigned long picSize, int *headLen){
	*sendBuffer=130;
	if( picSize < 126 ) {
		*(sendBuffer+1)=picSize;
		*headLen=2;
	} else if (picSize < 65537 ) { // 2 byte
		*(sendBuffer+1)=126;
		*(sendBuffer+2)= ((picSize & 0xff00) >> 8);
		*(sendBuffer+3)= (picSize & 0xff);
		*headLen=4;
	} else {
		*(sendBuffer+1)=127;
		*(sendBuffer+2)=0;
		*(sendBuffer+3)=0;
		*(sendBuffer+4)=0;
		*(sendBuffer+5)=0;
		*(sendBuffer+6)= ((picSize & 0xff000000) >> 24);
		*(sendBuffer+7)= ((picSize & 0xff0000) >> 16);
		*(sendBuffer+8)= ((picSize & 0xff00) >> 8);
		*(sendBuffer+9)= (picSize & 0xff);
		*headLen=10;
	}
	return 0;
}

int sendImage(int clientFd) {
	int imgFd;
	struct stat imgSt;
	unsigned int readSize;
	unsigned long remainSize;
	int headLen=0;
	unsigned char txBuf[MAX_TX_LEN];
	struct timeval completeTime;

	stat(IMAGE_NAME, &imgSt);
	remainSize=imgSt.st_size;

	imgFd = open(IMAGE_NAME, O_RDONLY);
	if(imgFd < 0) {
	   printf("Error Opening Image File");
	   return -1;
	} 
	
	memset(txBuf, 0, MAX_TX_LEN);
	headLen=0;

	fromWebSocketHeader(txBuf, remainSize, &headLen);
	printf("headLen=%d\n",headLen);

	if( remainSize > (MAX_TX_LEN-headLen) )
		readSize = read(imgFd, txBuf + headLen, MAX_TX_LEN-headLen);
	else
		readSize = read(imgFd, txBuf + headLen, remainSize);
	remainSize -= readSize;
	readSize += headLen;
    write(clientFd, txBuf, readSize);                        
	
	while(remainSize) {
		memset(txBuf, 0, MAX_TX_LEN);
		if ( remainSize >= MAX_TX_LEN) {
			readSize = read(imgFd, txBuf, MAX_TX_LEN);
		} else {
			readSize = read(imgFd, txBuf, remainSize);
		}
    	write(clientFd, txBuf, readSize);                        
		remainSize -= readSize;
	}

	gettimeofday(&completeTime , NULL);
	if(DEBUG)
		printf("File size: %ld bytes, time=%ld.%ld\n", imgSt.st_size, completeTime.tv_sec, completeTime.tv_usec);

	if ( imgFd )
		close(imgFd);

	return 0;
}

int sendcontiImg(int clientFd) {
	sendImage(clientFd);
	while( !recvAck(clientFd) ) {
	   printf("recvAck\n");
		usleep(TRANS_INTERVAL *1000);
		sendImage(clientFd);
	}
	return 0;
}

char * calculateSHA1(char * data) {
	char * sha1DataTemp;
	char * sha1Data;
	int i, n=0;
	sha1DataTemp=sha1_hash(data);
	n=strlen(sha1DataTemp); 
	sha1Data=(char *)malloc(n/2+1);
	memset(sha1Data,0,n/2+1);
	
	for(i=0;i<n;i+=2)
	{
	  sha1Data[i/2]=htoi(sha1DataTemp,i,2);
	}
	return sha1Data;
}

char * fetchSecKey()  
{  
	static char key[MAX_WEB_SOCKET_KEY_LEN];
	char *keyBegin;
	char *flag="Sec-WebSocket-Key: ";
	int i=0, bufLen=0;
	memset(key, 0, MAX_WEB_SOCKET_KEY_LEN);
	keyBegin=strstr(rxBuf,flag);
	if(!keyBegin){
		return NULL;
	}
	keyBegin+=strlen(flag);

	bufLen=strlen(rxBuf);
	for(i=0;i<bufLen;i++){
		if( (keyBegin[i]==0x0A) || (keyBegin[i]==0x0D) ) {
			break;
		}
		key[i]=keyBegin[i];
	}
	return key;
}

char * calculateKey() {
	char * clientKey;
	const char * GUID="258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	char * sha1Result;
	clientKey=fetchSecKey();
	if(!clientKey) {
		return NULL;
	}
	strcat(clientKey,GUID);
	sha1Result=calculateSHA1(clientKey);
	return base64_encode(sha1Result, strlen(sha1Result));
}

int handshake(int clientFd)
{
	if(!clientFd)
		return -1;

	memset(txBuf, 0, MAX_TX_LEN);
	sprintf(txBuf, "HTTP/1.1 101 Switching Protocols\r\n");  
	sprintf(txBuf, "%sUpgrade: websocket\r\n", txBuf);  
	sprintf(txBuf, "%sConnection: Upgrade\r\n", txBuf);  
	sprintf(txBuf, "%sSec-WebSocket-Accept: %s\r\n\r\n", txBuf, calculateKey());  
	 
	printf("Response Header:%s\n",txBuf);  
	
	write(clientFd,txBuf,strlen(txBuf));  
	return 0;
}

int main(int argc, char *argv[]) {
	int port=DEFEULT_SERVER_PORT;
	int serverFd, clientFd;
	struct sockaddr_in servAddr, cliAddr;
	socklen_t cliAddrLen;
	int flag=1, readCnt;
	if( argc > 1 ) {
		port=atoi(argv[1]);
	}
	if(port <=0 || port > 0xFFFF) {
		printf("Port(%d) is out of range(1-%d)\n",port,0xFFFF);  
		usage(argv[0]);
		return -1;  
	}
	serverFd = socket(AF_INET, SOCK_STREAM, 0);  
	
	bzero(&servAddr, sizeof(servAddr));  
	servAddr.sin_family = AF_INET;  
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);  
	servAddr.sin_port = htons(port);  
	if( setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1 ) {
		printf("set socket options error.\n");
		return -1;
	}

	bind(serverFd, (struct sockaddr *)&servAddr, sizeof(servAddr));  
	listen(serverFd, 1);
	cliAddrLen=sizeof(cliAddr);
	printf("Waiting for connection at Port(%d)...\n", port);
	clientFd=accept(serverFd, (struct sockaddr *)&cliAddr, &cliAddrLen);
	printf("From %s at PORT %d\n",  inet_ntoa(cliAddr.sin_addr),  ntohs(cliAddr.sin_port));
	memset(rxBuf, 0, MAX_RX_LEN);

	readCnt=read(clientFd, rxBuf, MAX_RX_LEN);
	printf("--------- Read Data From Client(%d) --------------\n", readCnt);
	printf("%s", rxBuf);
	printf("----------------------------------------------\n");
	handshake(clientFd);
	sendcontiImg(clientFd);

	return 0;
}

