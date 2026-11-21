savedcmd_et7480-audioswitch.mod := printf '%s\n'   et7480-audioswitch.o | awk '!x[$$0]++ { print("./"$$0) }' > et7480-audioswitch.mod
