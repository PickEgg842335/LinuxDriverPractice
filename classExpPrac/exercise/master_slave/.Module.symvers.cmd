cmd_/root/exercise/master_slave/Module.symvers :=  sed 's/ko$$/o/'  /root/exercise/master_slave/modules.order | scripts/mod/modpost -m      -o /root/exercise/master_slave/Module.symvers -e -i Module.symvers -T - 