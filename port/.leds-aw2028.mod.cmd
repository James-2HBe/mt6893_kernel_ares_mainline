savedcmd_leds-aw2028.mod := printf '%s\n'   leds-aw2028.o | awk '!x[$$0]++ { print("./"$$0) }' > leds-aw2028.mod
