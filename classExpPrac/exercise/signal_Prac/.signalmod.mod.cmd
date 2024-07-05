cmd_/root/exercise/signal_Prac/signalmod.mod := printf '%s\n'   signalmod.o | awk '!x[$$0]++ { print("/root/exercise/signal_Prac/"$$0) }' > /root/exercise/signal_Prac/signalmod.mod
