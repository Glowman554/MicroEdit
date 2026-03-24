SRCS = $(shell find -name '*.[cS]')
OBJS = $(addsuffix .o,$(basename $(SRCS)))


CFLAGS = -O2 -Wall -Iinclude -g
LDFLAGS =
PROGRAM ?= microedit

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

install: $(PROGRAM)
	cp $(PROGRAM) ~/go/bin/$(PROGRAM)

%.o: %.c
	@echo CC $^
	@$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.S
	@echo AS $^
	@$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJS) $(PROGRAM) 
	
compile_flags.txt:
	deno run -A https://raw.githubusercontent.com/Glowman554/MicroOS/refs/heads/master/compile_flags.ts $(CFLAGS) > compile_flags.txt