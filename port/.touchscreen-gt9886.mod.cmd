savedcmd_touchscreen-gt9886.mod := printf '%s\n'   touchscreen-gt9886.o | awk '!x[$$0]++ { print("./"$$0) }' > touchscreen-gt9886.mod
