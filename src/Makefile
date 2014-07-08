DRIVER=uif-bsl
MSP=MSP430F1611

CC=msp430-gcc -mmcu=$(MSP)
CFLAGS= -O3 -Wall -DUSE_DMA #-DDAC_TB
LDFLAGS= -O3

OUT=fw
SRC = main.c adc.c rf.c dma.c dac.c usb_serial.c
OBJS=$(SRC:%.c=%.o)
DEBUG=0

ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG
endif

ifeq ($(TX),1)
CFLAGS += -DTX
else
CFLAGS += -DRX
endif

default: all

all: .dep $(OUT)

debug:
	make DEBUG=1

tx:
	make TX=1

rx:
	make TX=0

debug_tx:
	make DEBUG=1 TX=1

debug_rx:
	make DEBUG=1 TX=0

$(OUT): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^
	msp430-objcopy --output-target=ihex $(OUT) main.ihex

clean:
	@rm -f *.o .dep $(OUT) main.ihex

.dep:
	$(CC) -MM $(SRC) > .dep

upload: $(OUT)
	mspdebug $(DRIVER) "prog fw"

bootload: $(OUT)
	#/Users/fabian/smeagol/tinyos-main/tools/usr/local/bin/tos-bsl --telosb -c /dev/tty.usbserial-XBQTBXPG -r -e -I -p main.ihex
	/Users/fabian/smeagol/tinyos-main/tools/usr/local/bin/tos-bsl --telosb -c /dev/tty.usbserial-XBR5W5YZ -r -e -I -p main.ihex

bootload2: $(OUT)
	/Users/fabian/smeagol/tinyos-main/tools/usr/local/bin/tos-bsl --telosb -c /dev/tty.usbserial-M4AJ0EMQ -r -e -I -p main.ihex

bootload3: $(OUT)
	/Users/fabian/smeagol/tinyos-main/tools/usr/local/bin/tos-bsl --telosb -c /dev/tty.usbserial-XBQTBXPG -r -e -I -p main.ihex

-include .dep
.PHONY: clean upload .dep debug
