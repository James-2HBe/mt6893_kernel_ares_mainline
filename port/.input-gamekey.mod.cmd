savedcmd_input-gamekey.mod := printf '%s\n'   input-gamekey.o | awk '!x[$$0]++ { print("./"$$0) }' > input-gamekey.mod
