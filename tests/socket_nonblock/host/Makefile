
all:

EXE = ./socket_nonblock

$(EXE): ../socket_nonblock.c

CFLAGS += -g -Wall -Wextra

.PHONY: all run clean
all: $(EXE)
	@echo \"make run\" to start.

run: $(EXE)
	$(EXE)

$(EXE):
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(EXE) 

