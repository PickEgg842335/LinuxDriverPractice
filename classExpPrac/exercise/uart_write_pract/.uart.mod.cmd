cmd_/root/exercise/uart_write_pract/uart.mod := printf '%s\n'   uart.o | awk '!x[$$0]++ { print("/root/exercise/uart_write_pract/"$$0) }' > /root/exercise/uart_write_pract/uart.mod
