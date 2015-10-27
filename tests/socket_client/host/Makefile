
.PHONY: all run clean
all: server
	@echo \"make run\" to start server.

run: server
	./$<

server.c: ../../socket_server/server.c
	cp $< $@

clean:
	rm server server.c

