
CPPFLAGS += -DSERVER_IP_ADDR="\"192.168.1.99\""
CPPFLAGS += -DSERVER_PORT=8888

.PHONY: all run clean
all: client
	@echo \"make run\" to start client.

run: client
	./$<

client.c: ../../socket/socket_client_test.c
	cp $< $@

clean:
	rm client

