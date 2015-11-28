
.PHONY: all run clean
all: server
	@echo \"make run\" to start server.

run: server
	./$<

server.c: ../../udp_server/udp_server.c
	cp $< $@

clean:
	rm server server.c

