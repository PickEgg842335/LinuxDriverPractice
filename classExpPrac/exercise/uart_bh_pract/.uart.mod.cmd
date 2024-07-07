cmd_/root/exercise/uart_bh_pract/uart.mod := printf '%s\n'   uart_ring.o ringbuf.o | awk '!x[$$0]++ { print("/root/exercise/uart_bh_pract/"$$0) }' > /root/exercise/uart_bh_pract/uart.mod
