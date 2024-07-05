cmd_/root/exercise/chardev/chardev.mod := printf '%s\n'   chardev.o | awk '!x[$$0]++ { print("/root/exercise/chardev/"$$0) }' > /root/exercise/chardev/chardev.mod
