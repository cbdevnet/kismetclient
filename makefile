.PHONY: clean

LIBS=
CFLAGS=-Wall

all: kismetcli

kismetcli: kismetcli.c

clean:
	rm kismetcli
