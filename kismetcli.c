#include "kismetcli.h"

#define PROTO_CMD "!0 ENABLE SSID *\n!0 ENABLE STATUS *\n!0 ENABLE CLIENT *\n"

void process_line(int fd, char* data){
	//printf("DATA %d: %s\n",strlen(data), data);

	if(!strncmp(data, "*KISMET", 7)){
		printf("Enabling protocols\n");
		send(fd, PROTO_CMD, strlen(PROTO_CMD), 0);
	}

	if(!strncmp(data, "*TIME", 5)){
		printf("TIME %d: %s\n",strlen(data), data);
	}
}

int main(int argc, char** argv){
	int sock;
	struct readline_r rr;
	int error, bytes;
	char* data;

	sock=sock_connect("129.13.215.25", 2501);
	if(sock<0){
		return 1;
	}

	error=sock_next_line(sock, NULL, &rr);
	if(error<0){
		return 1;
	}

	do{
		error=sock_next_line(sock, &data, &rr);
		switch(error){
			case 0:
			case -1:
				//somethings not right
				perror("sock_next");
				break;
			default:
				process_line(sock, data);
		}
	}while(error>0);

	//clean up
	sock_next_line(-1, NULL, &rr);

	return 0;
}
