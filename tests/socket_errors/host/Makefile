
#CPPFLAGS += -DSERVER_IP_ADDR="\"192.168.1.99\""

.PHONY: all run clean
all: socket_errors
	@echo \"make run\" to start client.

run: socket_errors
	./$<

socket_errors.c: ../socket_errors.c
	cp $< $@

clean:
	rm socket_errors socket_errors.c

