savedcmd_st21nfc.ko := ld.lld -r -EL  -maarch64elf -z norelro -z noexecstack --build-id=sha1  -T /workspace/linux-7.3.3/scripts/module.lds -o st21nfc.ko st21nfc.o st21nfc.mod.o .module-common.o
