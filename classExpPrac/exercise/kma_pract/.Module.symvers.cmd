cmd_/root/exercise/kma_pract/Module.symvers :=  sed 's/ko$$/o/'  /root/exercise/kma_pract/modules.order | scripts/mod/modpost -m      -o /root/exercise/kma_pract/Module.symvers -e -i Module.symvers -T - 
