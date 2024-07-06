cmd_/root/exercise/uart_interrupt_Pract/uart.mod := printf '%s\n'   uart.o | awk '!x[$$0]++ { print("/root/exercise/uart_interrupt_Pract/"$$0) }' > /root/exercise/uart_interrupt_Pract/uart.mod
