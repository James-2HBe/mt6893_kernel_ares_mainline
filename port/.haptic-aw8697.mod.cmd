savedcmd_haptic-aw8697.mod := printf '%s\n'   haptic-aw8697.o | awk '!x[$$0]++ { print("./"$$0) }' > haptic-aw8697.mod
