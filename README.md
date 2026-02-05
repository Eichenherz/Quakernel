# Quakernel

A minimal kernel designed to run Quake with sw rendering.

Based on limine-c-template.

## PRE-REQ:
 sudo apt install -y build-essential clang llvm lld mtools qemu-system-riscv64

## RUN:
 Build Image: make TOOLCHAIN=llvm build-hdd ( llvm is default so put anything you want there )
 Example run: ./run_qemu.sh Images/Quakernel_ReleaseWithDebugInfo_riscv64.hdd

## TODO
