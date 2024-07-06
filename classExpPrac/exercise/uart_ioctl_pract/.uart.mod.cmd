cmd_/root/exercise/uart_ioctl_pract/uart.mod := printf '%s\n'   uart.o | awk '!x[$$0]++ { print("/root/exercise/uart_ioctl_pract/"$$0) }' > /root/exercise/uart_ioctl_pract/uart.mod
