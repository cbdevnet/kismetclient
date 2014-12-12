#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define _DEFAULT_RECV_CHUNK 1024

int sock_connect(char* host, uint16_t port){
	int sockfd=-1, error;
	char port_str[20];
	struct addrinfo hints;
	struct addrinfo* head;
	struct addrinfo* iter;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	snprintf(port_str, sizeof(port_str), "%d", port);

	error=getaddrinfo(host, port_str, &hints, &head);
	if(error){
		fprintf(stderr, "getaddrinfo: %s\r\n", gai_strerror(error));
		return -1;
	}

	for(iter=head;iter;iter=iter->ai_next){
		sockfd=socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol);
		if(sockfd<0){
			continue;
		}

		error=connect(sockfd, iter->ai_addr, iter->ai_addrlen);
		if(error!=0){
			close(sockfd);
			continue;
		}

		break;
	}

	freeaddrinfo(head);
	iter=NULL;

	if(sockfd<0){
		perror("socket");
		return -1;
	}

	if(error!=0){
		perror("connect");
		return -1;
	}

	return sockfd;
}

struct readline_r {
	int fd;
	bool sock_active;
	char* data_buffer;
	unsigned data_length;
	unsigned data_offset;
	unsigned proc_offset;
};

int sock_next_line(int fd, char** out, struct readline_r* info){
	int bytes;
	unsigned i;

	if(!info){
		return -1;
	}
	
	if(!out&&fd>0){
		info->fd=fd;
		info->data_buffer=calloc(2*_DEFAULT_RECV_CHUNK, sizeof(char));
		info->data_length=2*_DEFAULT_RECV_CHUNK;
		info->data_offset=0;
		info->sock_active=true;
		info->proc_offset=0;
		return (info->data_buffer==NULL)?-1:0;
	}
	
	if(!out&&fd<0){
		//clean up
		if(info->data_buffer){
			free(info->data_buffer);
		}
		return 0;
	}
	
	if(out&&info->fd!=fd){
		//struct does not belong to this socket
		return -1;
	}

	if(info->proc_offset){
		//copy back
		for(i=0;i+info->proc_offset<info->data_offset;i++){
			info->data_buffer[i]=info->data_buffer[i+info->proc_offset];
		}
		info->data_offset-=info->proc_offset;
		info->proc_offset=0;
	}

	//check if buffer contains another sentence
	for(i=0;i<info->data_offset;i++){
		if(info->data_buffer[i]=='\n'){
			//handle
			info->data_buffer[i]=0;
			info->proc_offset=i+1;
			(*out)=info->data_buffer;
			return i;
		}
	}

	if(!info->sock_active){
		//check if data left but not terminated
		if(info->data_offset){
			info->data_buffer[info->data_offset]=0;
			info->proc_offset=info->data_offset; //this is intentional
			(*out)=info->data_buffer;
			return info->proc_offset;
		}
		return 0;
	}

	//receive until newline was read
	do{
		//realloc if buffer too small
		if(info->data_length-info->data_offset<_DEFAULT_RECV_CHUNK){
			info->data_buffer=realloc(info->data_buffer, (info->data_length+_DEFAULT_RECV_CHUNK)*sizeof(char));
			if(!info->data_buffer){
				return -1;
			}
			memset(info->data_buffer+info->data_length, 0, _DEFAULT_RECV_CHUNK);
			info->data_length+=_DEFAULT_RECV_CHUNK;
		}

		bytes=recv(info->fd, info->data_buffer+info->data_offset, info->data_length-info->data_offset-1, 0);
	
		if(bytes>0){
			//handle
			for(i=0;i<bytes;i++){
				if(info->data_buffer[info->data_offset+i]=='\n'){
					info->data_buffer[info->data_offset+i]=0;
					info->proc_offset=info->data_offset+i+1;
					(*out)=info->data_buffer;
					break;
				}
			}
			info->data_offset+=bytes;
		}
		else if(bytes<0){
			//error, check errno.
			return -1;
		}
		else{
			info->data_buffer[info->data_offset]=0;
			info->proc_offset=info->data_offset+1;
			(*out)=info->data_buffer;
			info->sock_active=false;
		}
	}while(info->proc_offset<1);

	//return bytes in sentence
	return info->proc_offset-1;
}
