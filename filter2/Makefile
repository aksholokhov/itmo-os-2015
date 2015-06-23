CC = gcc
CC_OPTS = -Wall -g -O2 -std=c99 -I../lib
LD_OPTS = -L../lib -lhelpers -lbufio

PROGRAM = filter2

all: $(PROGRAM)

clean:
	rm -f $(PROGRAM)
	rm -f $(PROGRAM).o

$(PROGRAM): $(PROGRAM).o ../lib/libbufio.so ../lib/libhelpers.so
	$(CC) -o $@ $< $(LD_OPTS)

$(PROGRAM).o: $(PROGRAM).c
	$(CC) -o $@ -c $< $(CC_OPTS)

../lib/libbufio.so:
	$(MAKE) -C $(dir $@) $(notdir $@)

../lib/libhelpers.so:
	$(MAKE) -C $(dir $@) $(notdir $@)


