.PHONY: clean

LIBS=
CFLAGS=-Wall -g

all: kismetcli

kismetcli: kismetcli.c

clean:
	rm kismetcli
