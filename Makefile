.PHONY: all clean

include Config.mk

all: $(OBJS) tmx.so
	$(AR) rcs $(OUT) $(OBJS)

tmx.so: $(TMX_OBJS)
	$(AR) rcs $(TMX_OUT) $(TMX_OBJS)

%: %.c
	$(CC) -c $(CFLAGS) $(LIBS) -o $@ $<

clean:
	rm -f $(OBJS)
	rm -f $(TMX_OBJS)
	rm -f $(TMX_OUT)
	rm -f $(OUT)
