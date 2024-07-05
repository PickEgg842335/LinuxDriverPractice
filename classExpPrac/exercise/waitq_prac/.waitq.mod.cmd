cmd_/root/exercise/waitq_prac/waitq.mod := printf '%s\n'   waitq.o | awk '!x[$$0]++ { print("/root/exercise/waitq_prac/"$$0) }' > /root/exercise/waitq_prac/waitq.mod
