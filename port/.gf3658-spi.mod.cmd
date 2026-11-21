savedcmd_gf3658-spi.mod := printf '%s\n'   gf3658-spi.o | awk '!x[$$0]++ { print("./"$$0) }' > gf3658-spi.mod
