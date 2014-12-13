#include "kismetcli.h"

int setup_protocols(char* line, PROCDATA* params);
int print_line(char* line, PROCDATA* params);

char* protocols[]={
	"!0 ENABLE STATUS *\n",
	"!0 ENABLE CLIENT *\n",
	"!0 ENABLE SSID *\n"
};

HANDLER protocol_handlers[]={
	{"*KISMET: ", setup_protocols},
	{"*SSID: ", print_line},
	{"", NULL}
};

int resolve_handler(char* data, PROCDATA* procdata){
	unsigned i;
	for(i=0;i<sizeof(protocol_handlers)/sizeof(HANDLER);i++){
		if(!strncmp(data, protocol_handlers[i].header, strlen(protocol_handlers[i].header))){
			return protocol_handlers[i].handler(data, procdata);
		}
	}
	fprintf(stderr, "Unhandled line.\n");
	return 0;
}
	
int setup_protocols(char* line, PROCDATA* params){
	unsigned i;
	
	fprintf(stderr, "Enabling protocols\n");

	for(i=0;i<sizeof(protocols)/sizeof(char*);i++){
		send(params->fd, protocols[i], strlen(protocols[i]), 0);
	}
}

int print_line(char* data, PROCDATA* params){
	printf("DATA %d: %s\n",strlen(data), data);
}

int main(int argc, char** argv){
	int sock;
	struct readline_r rr;
	int error, bytes;
	char* data;
	PROCDATA procdata;

	sock=sock_connect("129.13.215.25", 2501);
	if(sock<0){
		return 1;
	}

	procdata.fd=sock;

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
				if(resolve_handler(data, &procdata)<0){
					fprintf(stderr, "Handler failed, aborting\n");
					break;
				}
		}
	}while(error>0);

	//clean up
	sock_next_line(-1, NULL, &rr);

	return 0;
}
