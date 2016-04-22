LAYOUT ?= qwerty_alt
USB ?= /dev/ttyU0

MCU=atmega32u4
F_CPU=16000000
TARGET=atreus
SWAPCOLUMNS=no

build: ${TARGET}.hex

upload: ${TARGET}.hex
	while [ ! -r ${USB} ]; do sleep 1; done; \
	avrdude -p ${MCU} -c avr109 -U flash:w:${TARGET}.hex -P ${USB}

clean:
	@rm -f ${TARGET} ${TARGET}.hex *.o

layout.h: ${LAYOUT_DEPENDS}
	@cp -n layout_${LAYOUT}.h layout.h

${TARGET}.o: layout.h
usb_keyboard.o: usb_keyboard.h

${TARGET}: ${TARGET}.o usb_keyboard.o
	avr-gcc -mmcu=${MCU} usb_keyboard.o ${TARGET}.o -o ${TARGET}

${TARGET}.hex: ${TARGET}
	avr-size ${TARGET}
	avr-objcopy -O ihex -R .eeprom ${TARGET} ${TARGET}.hex

.c.o:
.if "${SWAPCOLUMNS}" == "yes"
	avr-gcc -std=gnu99 -DSWAPCOLUMNS=${SWAPCOLUMNS} -Os -D F_CPU=${F_CPU}UL -mmcu=${MCU} -c -o $@ $<
.else
	avr-gcc -std=gnu99 -Os -D F_CPU=${F_CPU}UL -mmcu=${MCU} -c -o $@ $<
.endif
