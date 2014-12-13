#include "kismetcli.h"

int setup_protocols(char* line, PROCDATA* params);
int print_line(char* line, PROCDATA* params);
int print_ssid(char* line, PROCDATA* params);
int print_time(char* line, PROCDATA* params);

HANDLER handlers[]={
	{NULL, "*KISMET: ", setup_protocols},
	{NULL, "*TIME: ", print_time},
	{"!0 ENABLE CLIENT *\n", "*CLIENT: ", NULL},
	{"!0 ENABLE SSID *\n", "*SSID: ", print_ssid},
	{"!0 ENABLE STATUS *\n", "*STATUS: ", NULL},
	{NULL, "", print_line}
};

int resolve_handler(char* data, PROCDATA* procdata){
	unsigned i;
	for(i=0;i<sizeof(handlers)/sizeof(HANDLER);i++){
		if(handlers[i].header){
			if(!strncmp(data, handlers[i].header, strlen(handlers[i].header))){
				if(handlers[i].handler){
					return handlers[i].handler(data, procdata);
				}
				else{
					return 0;
				}
			}
		}
	}
	fprintf(stderr, "No handler found for line: %s\n", data);
	return -1;
}
	
int setup_protocols(char* line, PROCDATA* params){
	unsigned i;
	
	fprintf(stderr, "Enabling protocols\n");

	for(i=0;i<sizeof(handlers)/sizeof(HANDLER);i++){
		if(handlers[i].enable){
			send(params->fd, handlers[i].enable, strlen(handlers[i].enable), 0);
		}
	}
	return 0;
}

int print_line(char* data, PROCDATA* params){
	printf("DATA %d: %s\n",strlen(data), data);
	return 0;
}

int print_ssid(char* data, PROCDATA* params){
	printf("Some SSID data\n");
	return 0;
}

int print_time(char* data, PROCDATA* params){
	printf("TIME %s\n",data+7);
	return 0;
}

int main(int argc, char** argv){
	int sock;
	struct readline_r rr;
	int error;
	char* data;
	PROCDATA procdata;

	sock=sock_connect("127.0.0.1", 2501);
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
				resolve_handler(data, &procdata);
		}
	}while(error>0);

	//clean up
	sock_next_line(-1, NULL, &rr);

	return 0;
}
