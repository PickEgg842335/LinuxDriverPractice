cmd_/root/exercise/uart_read_Prac/uart.mod := printf '%s\n'   uart.o | awk '!x[$$0]++ { print("/root/exercise/uart_read_Prac/"$$0) }' > /root/exercise/uart_read_Prac/uart.mod
