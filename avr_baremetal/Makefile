# ==============================================================================
# Makefile for Bare-Metal ATmega328P Smart Shoe Firmware
# ==============================================================================

MCU          = atmega328p
F_CPU        = 16000000UL
CC           = avr-gcc
OBJCOPY      = avr-objcopy
AVRDUDE      = avrdude
PROGRAMMER   = arduino
PORT         = COM3
BAUD         = 115200

CFLAGS       = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=c99
TARGET       = smart_shoe_baremetal

SRCS         = main.c
OBJS         = $(SRCS:.c=.o)

all: $(TARGET).hex

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	@echo "Build complete: $(TARGET).hex"

flash: $(TARGET).hex
	$(AVRDUDE) -v -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:$(TARGET).hex:i

clean:
	rm -f *.o *.elf *.hex

.PHONY: all clean flash
