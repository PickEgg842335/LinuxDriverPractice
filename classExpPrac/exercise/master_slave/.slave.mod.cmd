cmd_/root/exercise/master_slave/slave.mod := printf '%s\n'   slave.o | awk '!x[$$0]++ { print("/root/exercise/master_slave/"$$0) }' > /root/exercise/master_slave/slave.mod
