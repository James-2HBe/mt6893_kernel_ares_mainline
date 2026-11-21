savedcmd_st21nfc.mod := printf '%s\n'   st21nfc.o | awk '!x[$$0]++ { print("./"$$0) }' > st21nfc.mod
