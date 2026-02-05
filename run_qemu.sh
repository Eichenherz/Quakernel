#!/usr/bin/env bash
set -euo pipefail

#ARCH="${1:-riscv64}"
ARCH="riscv64"
IMAGE="${1:?Error: IMAGE is required as third argument}"


# Select QEMU binary per arch
QEMU_BIN="qemu-system-$ARCH"
# Default user QEMU flags. These are appended to the QEMU command calls.
QEMUFLAGS="-m 4G"

if [[ "${2:-}" == "--debug" ]]; then
  QEMUFLAGS+=" -S -gdb tcp::1234 -serial mon:stdio" # DON'T forget the leading space
  echo "QEMU_GDB_READY tcp::1234"
fi

case "$ARCH" in
    riscv64) MACH="-M virt -cpu rv64" ;;
    #loongarch64) MACH="-M virt -cpu la464 " ;;
    #aarch64) MACH="-M virt -cpu cortex-a72" ;;
    #x86_64) MACH="-M q35" ;;
    *) echo "Unsupported arch: $ARCH" && exit 1 ;;
esac

# Devices for virt machines (except x86_64)
DEVICES="-device ramfb -device qemu-xhci -device usb-kbd -device usb-mouse"
if [[ "$ARCH" == "x86_64" ]]; then
    DEVICES=""
fi

rm -rf edk2-ovmf
curl -L https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/edk2-ovmf.tar.gz | gunzip | tar -xf -

# UEFI firmware (OVMF)
OVMF_CODE_FD="edk2-ovmf/ovmf-code-$ARCH.fd"


MEDIA="${IMAGE##*.}" 
if [[ "$MEDIA" == "iso" ]]; then
    [ -f "$IMAGE" ] || { echo "Missing $IMAGE. Run: make $IMAGE"; exit 1; }
    DRIVE="-cdrom $IMAGE"
elif [[ "$MEDIA" == "hdd" ]]; then
    [ -f "$IMAGE" ] || { echo "Missing $IMAGE. Run: make $IMAGE"; exit 1; }
    DRIVE="-drive file=$IMAGE,format=raw,if=virtio"
else
    echo "Invalid MEDIA: $MEDIA (expected 'iso' or 'hdd')"
    exit 1
fi

# Assemble final QEMU command
exec $QEMU_BIN \
    $MACH \
    $DEVICES \
    -drive if=pflash,unit=0,format=raw,file="$OVMF_CODE_FD",readonly=on \
    $DRIVE \
    ${QEMUFLAGS:-}
