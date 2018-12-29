.PHONY: all clean

include config.mk

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(OUT)

%: %.c
	$(CC) -c $(CFLAGS) $(LIBS) -o $@ $<

clean:
	rm -f $(OBJS)
	rm -f $(OUT)
