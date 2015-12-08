
CFLAGS += -g -Wall -Wextra

.PHONY: all run clean
all: dns
	@echo \"make run\" to start.

run: dns
	./$<

dns.c: ../dns.c
	cp $< $@

dns: getaddrinfo.o

getaddrinfo.c: ../../../src/getaddrinfo.c
	cp $< $@

clean:
	rm -f dns dns.c

