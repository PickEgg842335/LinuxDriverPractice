cmd_/root/exercise/chardev/Module.symvers :=  sed 's/ko$$/o/'  /root/exercise/chardev/modules.order | scripts/mod/modpost -m      -o /root/exercise/chardev/Module.symvers -e -i Module.symvers -T - 
