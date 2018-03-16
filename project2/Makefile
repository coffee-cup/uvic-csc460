USER_LIB_PATH=$(realpath libs)
BOARD_TAG    = mega
BOARD_SUB    = atmega2560
ARDUINO_PORT = ${BASE_PORT}
MONITOR_BAUDRATE = 38400
MONITOR_CMD = miniterm.py --menu-char 27

ifdef TEST
	CFLAGS += -DRUN_TESTS
endif

include ${ARDMK_DIR}/Arduino.mk
