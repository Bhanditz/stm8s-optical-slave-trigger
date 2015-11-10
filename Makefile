TARGET = trigger.ihx
MCUFAM = stm8
MCUPART = stm8s105
MCUCFLAG = STM8S105
STM8FLASH = /home/roshan/bin/stm8flash

SRCS = main.c gpio.c timer.c state_machine.c
OBJS = $(SRCS:.c=.rel)
ASMS = $(SRCS:.c=.asm)
LSTS = $(SRCS:.c=.lst)
RSTS = $(SRCS:.c=.rst)
SYMS = $(SRCS:.c=.sym)
MAPS = $(SRCS:.c=.map)
MEMS = $(SRCS:.c=.mem)
ADBS = $(SRCS:.c=.adb)

CC = sdcc
LD = sdld
LIBS = STM8S105.lib stm8.lib
INCLUDES = -I/home/roshan/workspace/STM8S_StdPeriphLib/inc
LIBPATHS = -L/home/roshan/workspace/STM8S_StdPeriphLib/lib
CFLAGS = -m$(MCUFAM) -D$(MCUCFLAG) --Werror -c $(INCLUDES)
LFLAGS = -m$(MCUFAM) --out-fmt-ihx $(LIBPATHS)

.PHONY: all clean_objs clean flash

#all: $(TARGET) clean_objs
all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) -o $(TARGET) $(LFLAGS) $(OBJS) $(LIBS)

clean: clean_objs
	@$(RM) $(TARGET)
	@$(RM) $(TARGET:.ihx=.lk) $(TARGET:.ihx=.map)

clean_objs:
	@$(RM) $(OBJS)
	@$(RM) $(ASMS) $(LSTS) $(RSTS) $(SYMS) $(MAPS) $(MEMS) $(ADBS)

%.rel: %.c
	@$(CC) $(CFLAGS) $< -o $@

flash: $(TARGET)
	@sudo $(STM8FLASH) -c stlink -p $(MCUPART) -w $(TARGET)
