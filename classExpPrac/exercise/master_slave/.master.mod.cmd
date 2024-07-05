cmd_/root/exercise/master_slave/master.mod := printf '%s\n'   master.o | awk '!x[$$0]++ { print("/root/exercise/master_slave/"$$0) }' > /root/exercise/master_slave/master.mod
