cmd_/root/exercise/chardev_nonblock_Prac/nonblock.ko := ld -r -m elf_x86_64 -z noexecstack --no-warn-rwx-segments --build-id=sha1  -T arch/x86/module.lds -o /root/exercise/chardev_nonblock_Prac/nonblock.ko /root/exercise/chardev_nonblock_Prac/nonblock.o /root/exercise/chardev_nonblock_Prac/nonblock.mod.o;  true