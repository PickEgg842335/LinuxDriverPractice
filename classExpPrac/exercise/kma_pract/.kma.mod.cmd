cmd_/root/exercise/kma_pract/kma.mod := printf '%s\n'   kma.o | awk '!x[$$0]++ { print("/root/exercise/kma_pract/"$$0) }' > /root/exercise/kma_pract/kma.mod
