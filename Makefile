TARGET = trigger.ihx
MCUFAM = stm8
STM8FLASH = /home/roshan/bin/stm8flash

SRCS = main.c gpio.c timer.c adc.c state_machine.c
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.rel)
ASMS = $(SRCS:.c=.asm)
LSTS = $(SRCS:.c=.lst)
RSTS = $(SRCS:.c=.rst)
SYMS = $(SRCS:.c=.sym)
MAPS = $(SRCS:.c=.map)
MEMS = $(SRCS:.c=.mem)
ADBS = $(SRCS:.c=.adb)

include Make.defs

# Feature Defines
ifeq ($(DEBUG),y)
CFLAGS += -DDEBUG
endif

ifeq ($(WAKEUP_BUTTON),y)
CFLAGS += -DWAKEUP_BUTTON
endif

CC = sdcc
LD = sdld
LIBS += stm8.lib
INCLUDES += -I/home/roshan/workspace/STM8S_StdPeriphLib/inc
LIBPATHS += -L/home/roshan/workspace/STM8S_StdPeriphLib/lib
CFLAGS += -m$(MCUFAM) --Werror -c $(INCLUDES)
LFLAGS += -m$(MCUFAM) --out-fmt-ihx $(LIBPATHS)

.PHONY: all flash clean_objs clean clean_deps distclean

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) -o $(TARGET) $(LFLAGS) $(OBJS) $(LIBS)

flash: $(TARGET)
	@sudo $(STM8FLASH) -c stlink -p $(MCUPART) -w $(TARGET)

clean_objs:
	@$(RM) $(OBJS)
	@$(RM) $(ASMS) $(LSTS) $(RSTS) $(SYMS) $(MAPS) $(MEMS) $(ADBS)

clean: clean_objs
	@$(RM) $(TARGET)
	@$(RM) $(TARGET:.ihx=.lk) $(TARGET:.ihx=.map)

clean_deps:
	@$(RM) $(DEPS)

distclean: clean clean_deps
	@$(RM) -f config.h
	@$(RM) -f Make.defs

%.rel: %.c
	@$(CC) $(CFLAGS) $< -o $@

%.d: %.c
	@set -e; rm -f $@; \
         $(CC) -MM $(CFLAGS) $< > $@.$$$$; \
         sed 's,\($*\)\.rel[ :]*,\1.rel $@ : ,g' < $@.$$$$ > $@; \
         rm -f $@.$$$$

include $(DEPS)
