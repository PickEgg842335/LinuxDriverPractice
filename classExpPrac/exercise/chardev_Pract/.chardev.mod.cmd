cmd_/root/exercise/chardev_Pract/chardev.mod := printf '%s\n'   chardev.o | awk '!x[$$0]++ { print("/root/exercise/chardev_Pract/"$$0) }' > /root/exercise/chardev_Pract/chardev.mod
