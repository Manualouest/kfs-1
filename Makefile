######################################################
#						MAIN PART					 #
######################################################

NAME        := OScrat.iso

BUILD_DIR	:= build/
ISO_DIR     := $(BUILD_DIR)iso

KERNEL_BIN  := $(BUILD_DIR)kernel.bin

######################################################
#						CONFIG PART					 #
######################################################

CFG_DIR 	:= config/

LD          := ld
LINKER		:= $(CFG_DIR)linker.ld
LDFLAGS     := -m elf_i386

GCONF		:= $(CFG_DIR)grub.cfg

######################################################
#					COMPILATION PART				 #
######################################################

CC          := gcc
CFLAGS      := -m32 -ffreestanding -fno-builtin -fno-stack-protector \
               -nostdlib -nodefaultlibs -Wall -Wextra

INC_DIR		:= inc/

SRC_DIR		:= srcs/

C_SRCS      := kernel.c\
			   drivers/utils.c\
			   drivers/terminal.c\
			   drivers/devices/PS2/controller.c\
			   drivers/devices/PS2/mouse.c\
			   drivers/devices/PS2/keyboard.c

ASM_SRCS    := boot.s

vpath %.c $(SRC_DIR)
vpath %.s $(SRC_DIR)

OBJS        := $(C_SRCS:%.c=build/%.o) $(ASM_SRCS:%.s=build/%.o)

######################################################
#					DISPLAY PART					 #
######################################################

NOC			:= \e[0m\n

C1			:= \e[1m\e[38;5;150m
C2			:= \e[1m\e[38;5;151m
C3			:= \e[1m\e[38;5;152m
C4			:= \e[1m\e[38;5;153m

C5			:= \e[1m\e[38;5;37m
C6			:= \e[1m\e[38;5;30m
C7			:= \e[1m\e[38;5;29m

C8			:= \e[1m\e[38;5;110m
C9			:= \e[1m\e[38;5;109m

CE			:= \e[1m\e[38;5;1m

######################################################
#                   	RULES PART					 #
######################################################

all: $(NAME)

$(NAME): $(KERNEL_BIN) $(GCONF)
	@printf "$(C1)>>> Creating $(ISO_DIR)/boot/grub <<<$(NOC)"
	@mkdir -p $(ISO_DIR)/boot/grub
	@printf "$(C2)>>> Copy $(basename $(KERNEL_BIN)) <<<$(NOC)"
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	@printf "$(C3)>>> Copy $(basename $(GCONF)) <<<$(NOC)"
	@cp $(GCONF) $(ISO_DIR)/boot/grub/grub.cfg
	@printf "$(C4)>>> Creating $(NAME) <<<$(NOC)"
	@out=$$(grub-mkrescue -o $(NAME) $(ISO_DIR) 2>&1) || \
		{ echo "$(CE)$$out$(NOC)"; exit 1; }

$(KERNEL_BIN): $(OBJS) $(LINKER)
	@printf "$(C5)>>> Creating $(BUILD_DIR) <<<$(NOC)"
	@mkdir -p $(BUILD_DIR)
	@printf "$(C6)>>> Link objects files <<<$(NOC)"
	@$(LD) $(LDFLAGS) -T $(LINKER) -o $@ $(OBJS)
	@printf "$(C7)>>> Creating $(KERNEL_BIN) <<<$(NOC)"
	@grub-file --is-x86-multiboot $@ || \
		(echo "$(CE)Erreur: en-tête Multiboot invalide !$(NOC)" && exit 1)

run: $(NAME)
	@qemu-system-i386 -cdrom $(NAME)

clean:
	@printf "$(C8)>>> Cleaning $(BUILD_DIR) <<<$(NOC)"
	@rm -rf $(BUILD_DIR)

fclean: clean
	@printf "$(C9)>>> Cleaning $(NAME) <<<$(NOC)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re

######################################
#			BUILD RULES PART		 #
######################################

#@printf $(CR)"\e[1m\e[38;5;74m>>> $(EXEC) is created ! <<<\e[0m\n"

build/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) -I$(INC_DIR) $(CFLAGS) -c $< -o $@

build/%.o: %.s
	@mkdir -p $(dir $@)
	
	@$(CC) $(CFLAGS) -c $< -o $@

