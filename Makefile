NAME        := OScrat.iso
BUILD_DIR	:= build/
KERNEL_BIN  := $(BUILD_DIR)kernel.bin
ISO_DIR     := $(BUILD_DIR)iso

CC          := gcc
LD          := ld

CFG_DIR 	:= config/

LINKER		:= $(CFG_DIR)linker.ld
GCONF		:= $(CFG_DIR)grub.cfg

CFLAGS      := -m32 -ffreestanding -fno-builtin -fno-stack-protector \
               -nostdlib -nodefaultlibs -Wall -Wextra
LDFLAGS     := -m elf_i386 -T $(LINKER)

SRC_DIR		:= srcs/

C_SRCS      := kernel.c
ASM_SRCS    := boot.s

C_SRCS		:= $(addprefix $(SRC_DIR), $(C_SRCS))
ASM_SRCS	:= $(addprefix $(SRC_DIR), $(ASM_SRCS))


OBJS        := $(C_SRCS:%.c=build/%.o) $(ASM_SRCS:%.s=build/%.o)

all: $(NAME)

$(NAME): $(KERNEL_BIN) $(GCONF)
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	@cp $(GCONF) $(ISO_DIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(NAME) $(ISO_DIR)

$(KERNEL_BIN): $(OBJS) $(LINKER)
	@mkdir -p build
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@grub-file --is-x86-multiboot $@ || (echo "Erreur: en-tête Multiboot invalide !" && exit 1)

build/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.s
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

run: $(NAME)
	@qemu-system-i386 -cdrom $(NAME)

clean:
	@rm -rf $(BUILD_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all run clean fclean re
