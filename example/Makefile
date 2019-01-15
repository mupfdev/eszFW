.PHONY: all clean

include Config.mk

all: $(OBJS) $(LIBESZFW) $(LIBTMX)
	$(CC) $(CFLAGS) $(OBJS) $(LIBESZFW) $(LIBTMX) $(LIBS) -o $(OUT)

%: %.c
	$(CC) -c $(CFLAGS) $(LIBS) -o $@ $<

$(LIBESZFW):
	make -C ../

$(LIBTMX):
	make -C ../

clean:
	rm -f $(OBJS)
	rm -f $(OUT)
