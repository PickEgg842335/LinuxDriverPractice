cmd_/root/exercise/uart_interrupt_Pract/uart.ko := ld -r -m elf_x86_64 -z noexecstack --no-warn-rwx-segments --build-id=sha1  -T arch/x86/module.lds -o /root/exercise/uart_interrupt_Pract/uart.ko /root/exercise/uart_interrupt_Pract/uart.o /root/exercise/uart_interrupt_Pract/uart.mod.o;  true