.PHONY: all clean

include config.mk

all: $(OBJS) $(ESZFW)
	$(CC) $(CFLAGS) $(OBJS) $(ESZFW) $(LIBS) -o $(OUT)

%: %.c
	$(CC) -c $(CFLAGS) $(LIBS) -o $@ $<

$(ESZFW):
	make -C ../../

clean:
	rm -f $(OBJS)
	rm -f $(OUT)
