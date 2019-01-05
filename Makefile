.PHONY: all clean

include config.mk

all: $(OBJS)
	$(AR) rcs $(OUT) $(OBJS)

%: %.c
	$(CC) -c $(CFLAGS) $(LIBS) -o $@ $<

clean:
	rm -f $(OBJS)
	rm -f $(OUT)
