#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

//#include <netdb.h>
//#include <netinet/in.h>


int main(int argc, char *argv[])
{
		int sock;
		struct sockaddr_un server;
		char buf[1024];

		if (argc < 3) {
			printf("usage:%s <pathname> [binary|ascii]\n\n", argv[0]);
			exit(1);
		}

		// Open the socket
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sock < 0) {
			perror("opening stream socket");
			exit(1);
		}
		server.sun_family = AF_UNIX;
		strcpy(server.sun_path, argv[1]);

		bool isAscii = (strcmp(argv[2],"ascii") == 0) ? true : false;

		if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
			close(sock);
			perror("connecting stream socket");
			exit(1);
		}
		int readBytes;
		while(true) {
			bzero(buf,1024);
			readBytes = recv(sock,buf,1000, 0);
			if (readBytes < 0) {
			      perror("ERROR reading from socket");
			      exit(1);
			}
			if(isAscii) {
				buf[readBytes] = '\0';
				if(readBytes > 0) printf("%s \n",buf);
			} else {
				uint32_t *ptr = (uint32_t *)buf;
				while(((char *)ptr) <= ( buf+readBytes-1 )){
					if((*ptr & 0x80000000) != 0) { // this is an event
						printf("Event %d - ChipId = %d \n", (*ptr & 0x7FFFF), ((*ptr & 0x7FF80000) >> 19) );
					} else {
						printf("   ChipId = %d Hit(%d,%d) \n", ((*ptr & 0x7FF80000) >> 19),((*ptr & 0x0007FE00) >> 9), (*ptr & 0x000001FF) );
					}
					ptr++;
				}
			}
			usleep(250000);
		}
		exit(1);
}
