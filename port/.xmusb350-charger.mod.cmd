savedcmd_xmusb350-charger.mod := printf '%s\n'   xmusb350-charger.o | awk '!x[$$0]++ { print("./"$$0) }' > xmusb350-charger.mod
