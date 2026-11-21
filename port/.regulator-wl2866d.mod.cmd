savedcmd_regulator-wl2866d.mod := printf '%s\n'   regulator-wl2866d.o | awk '!x[$$0]++ { print("./"$$0) }' > regulator-wl2866d.mod
