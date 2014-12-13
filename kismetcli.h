#include <stdio.h>
#include <stdbool.h>

#include "sock_comms.c"

typedef struct /*_PROCESSING_DATA*/ {
	int fd;
} PROCDATA;

typedef int (*PROTOCOL_HANDLER)(char* , PROCDATA*);

typedef struct /*_PROTOCOL_HANDLER*/ {
	char* enable;
	char* header;
	PROTOCOL_HANDLER handler;
} HANDLER;
