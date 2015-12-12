
all:

EXE = ./gethostbyname_test

$(EXE): ../gethostbyname_test.c
$(EXE): ../../../src/getaddrinfo.c
$(EXE): ../../../src/gethostbyname.c

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

