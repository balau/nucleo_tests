.PHONY: all run clean
all: client
	@echo \"make run\" to start client.

run: client
	./$<

client.c: ../../socket/socket_client_test.c
	cp $< $@

clean:
	rm client

