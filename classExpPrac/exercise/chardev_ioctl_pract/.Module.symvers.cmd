cmd_/root/exercise/chardev_ioctl_pract/Module.symvers :=  sed 's/ko$$/o/'  /root/exercise/chardev_ioctl_pract/modules.order | scripts/mod/modpost -m      -o /root/exercise/chardev_ioctl_pract/Module.symvers -e -i Module.symvers -T - 