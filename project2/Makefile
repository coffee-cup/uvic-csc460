# USER_LIB_PATH=$(realpath ../common)

BOARD_TAG    = mega
BOARD_SUB    = atmega2560
ARDUINO_PORT = ${BASE_PORT}
include ${ARDMK_DIR}/Arduino.mk

switch:
	avr-gcc -c -O2 -DF_CPU=${CLOCK} -mmcu=${BOARD_SUB} -Wa,--gstabs -o cswitch.S.o cswitch.S
