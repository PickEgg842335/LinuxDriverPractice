cmd_/root/exercise/chardev_Practice2/chardev.mod := printf '%s\n'   chardev.o | awk '!x[$$0]++ { print("/root/exercise/chardev_Practice2/"$$0) }' > /root/exercise/chardev_Practice2/chardev.mod
