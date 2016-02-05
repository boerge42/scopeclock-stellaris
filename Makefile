TARGET = main


STELLARISWARE = ~/work/stellaris/stellarisware
SRC = $(wildcard *.c)
TOOLCHAIN = arm-none-eabi
PART = LM4F120H5QR
CPU = cortex-m4
FPU = fpv4-sp-d16
FABI = softfp

LINKER_FILE = main.ld

CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
CP = $(TOOLCHAIN)-objcopy
OD = $(TOOLCHAIN)-objdump
SZ = $(TOOLCHAIN)-size

CFLAGS = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CFLAGS+= -Os -ffunction-sections -fdata-sections
CFLAGS+= -MD -std=c99 -Wall 
#CFLAGS+= -pedantic
CFLAGS+= -DPART_$(PART) -c -DTARGET_IS_BLIZZARD_RA1
CFLAGS+= -g
CFLAGS+= -I $(STELLARISWARE)

LIB_GCC_PATH=$(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
LIBC_PATH=$(shell $(CC) $(CFLAGS) -print-file-name=libc.a)
LIBM_PATH=$(shell $(CC) $(CFLAGS) -print-file-name=libm.a)
LIBSW_PATH=$(STELLARISWARE)/driverlib/gcc-cm4f/*.o
LFLAGS = --gc-sections --entry ResetISR
CPFLAGS = -Obinary

ODFLAGS = -S

FLASHER=lm4flash
FLASHER_FLAGS=-v

OBJS = $(SRC:.c=.o)

#### Rules ####
all: $(OBJS) $(TARGET).axf $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	@echo

$(TARGET).axf: $(OBJS)
	$(LD) -T $(LINKER_FILE) $(LFLAGS) -o $(TARGET).axf $(OBJS) $(LIBM_PATH) $(LIBC_PATH) $(LIB_GCC_PATH) $(LIBSW_PATH)
	@echo

$(TARGET): $(TARGET).axf
	$(CP) $(CPFLAGS) $(TARGET).axf $(TARGET).bin
	$(OD) $(ODFLAGS) $(TARGET).axf > $(TARGET).lst
	@echo
	$(SZ) $(TARGET).axf
	@echo
	

flash: $(TARGET)
	$(FLASHER) $(TARGET).bin $(FLASHER_FLAGS)
	@echo

clean:
	rm *.o *.d *.bin *.lst *.axf
	@echo
	
