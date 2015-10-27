
CPPFLAGS += -DSERVER_IP_ADDR="\"192.168.1.99\""

.PHONY: all run clean
all: client
	@echo \"make run\" to start client.

run: client
	./$<

client.c: ../../socket_client/client.c
	cp $< $@

clean:
	rm client client.c

