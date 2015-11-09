
CPPFLAGS += -DSERVER_IP_ADDR="\"192.168.1.99\""
#CPPFLAGS += -DSERVER_IP_ADDR="\"192.168.1.173\""

.PHONY: all run clean
all: client
	@echo \"make run\" to start client.

run: client
	./$<

client.c: ../../udp_client/udp_client.c
	cp $< $@

clean:
	rm client client.c

