cmd_/root/exercise/work_queue_Pract/task_let.mod := printf '%s\n'   task_let.o | awk '!x[$$0]++ { print("/root/exercise/work_queue_Pract/"$$0) }' > /root/exercise/work_queue_Pract/task_let.mod
