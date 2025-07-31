#!/usr/bin/env bash
set -euo pipefail

ARCH="${1:-x86_64}"
MEDIA="${2:-iso}"


# Select QEMU binary per arch
QEMU_BIN="qemu-system-$ARCH"
# Default user QEMU flags. These are appended to the QEMU command calls.
QEMUFLAGS= "-m 4G"

case "$ARCH" in
    riscv64) MACH="-M virt -cpu rv64" ;;
    loongarch64) MACH="-M virt -cpu la464 " ;;
    aarch64) MACH="-M virt -cpu cortex-a72" ;;
    x86_64) MACH="-M q35" ;;
    *) echo "Unsupported arch: $ARCH" && exit 1 ;;
esac

# Devices for virt machines (except x86_64)
DEVICES="-device ramfb -device qemu-xhci -device usb-kbd -device usb-mouse"
if [[ "$ARCH" == "x86_64" ]]; then
    DEVICES=""
fi

# UEFI firmware (OVMF)
OVMF="ovmf/ovmf-code-$ARCH.fd"

rm -rf ovmf
mkdir -p ovmf
[ -f "$OVMF" ] || {
  curl -Lo "$OVMF" "https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-$ARCH.fd"
  case "$ARCH" in
    aarch64) dd if=/dev/zero of="$OVMF" bs=1 count=0 seek=67108864 status=none ;;
    riscv64) dd if=/dev/zero of="$OVMF" bs=1 count=0 seek=33554432 status=none ;;
  esac
}

# Image name
IMAGE_NAME="template-$ARCH"

# Media type
if [[ "$MEDIA" == "iso" ]]; then
    IMAGE="$IMAGE_NAME.iso"
    [ -f "$IMAGE" ] || { echo "Missing $IMAGE. Run: make $IMAGE"; exit 1; }
    DRIVE="-cdrom $IMAGE"
elif [[ "$MEDIA" == "hdd" ]]; then
    IMAGE="$IMAGE_NAME.hdd"
    [ -f "$IMAGE" ] || { echo "Missing $IMAGE. Run: make $IMAGE"; exit 1; }
    DRIVE="-hda $IMAGE"
else
    echo "Invalid MEDIA: $MEDIA (expected 'iso' or 'hdd')"
    exit 1
fi

# Assemble final QEMU command
exec $QEMU_BIN \
    $MACH \
    $DEVICES \
    -drive if=pflash,unit=0,format=raw,file="$OVMF",readonly=on \
    $DRIVE \
    ${QEMUFLAGS:-}
