cmd_/home/jason/lab/lab5/mymod/kshram.mod := printf '%s\n'   kshram.o | awk '!x[$$0]++ { print("/home/jason/lab/lab5/mymod/"$$0) }' > /home/jason/lab/lab5/mymod/kshram.mod
