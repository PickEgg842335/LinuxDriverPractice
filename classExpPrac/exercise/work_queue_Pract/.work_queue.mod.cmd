cmd_/root/exercise/work_queue_Pract/work_queue.mod := printf '%s\n'   work_queue.o | awk '!x[$$0]++ { print("/root/exercise/work_queue_Pract/"$$0) }' > /root/exercise/work_queue_Pract/work_queue.mod
