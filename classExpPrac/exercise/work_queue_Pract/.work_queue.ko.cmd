cmd_/root/exercise/work_queue_Pract/work_queue.ko := ld -r -m elf_x86_64 -z noexecstack --no-warn-rwx-segments --build-id=sha1  -T arch/x86/module.lds -o /root/exercise/work_queue_Pract/work_queue.ko /root/exercise/work_queue_Pract/work_queue.o /root/exercise/work_queue_Pract/work_queue.mod.o;  true
