SRC += features/led_comm.c
DEFERRED_EXEC_ENABLE = yes

CONSOLE_ENABLE = yes

ifeq ($(strip $(MOUSE_SIDE)), left)
    OPT_DEFS += -DIS_LEFT
endif
