#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/target/raspi-zero"
PORT_DIR="$ROOT/ports/raspi-zero"
mkdir -p "$BUILD_DIR"

arm-none-eabi-gcc \
  -mcpu=arm1176jzf-s \
  -mfpu=vfp \
  -mfloat-abi=hard \
  -ffreestanding \
  -fno-builtin \
  -fdata-sections \
  -ffunction-sections \
  -Os \
  -nostdlib \
  -Wl,--gc-sections \
  -T "$PORT_DIR/linker.ld" \
  "$PORT_DIR/boot.S" \
  "$PORT_DIR/frozen_blinky.c" \
  -o "$BUILD_DIR/kernel.elf"

arm-none-eabi-objcopy -O binary "$BUILD_DIR/kernel.elf" "$BUILD_DIR/kernel.img"

set +e
output="$(timeout 8s qemu-system-arm -M raspi0 -kernel "$BUILD_DIR/kernel.elf" -serial stdio -display none -no-reboot 2>&1)"
status=$?
set -e

printf '%s\n' "$output"
printf '%s\n' "$output" | grep 'RustScript Pi Zero bare-metal demo' >/dev/null
printf '%s\n' "$output" | grep 'OS: none' >/dev/null
printf '%s\n' "$output" | grep 'JIT: off' >/dev/null
printf '%s\n' "$output" | grep 'led:off' >/dev/null

if [[ "$status" != "0" && "$status" != "124" ]]; then
  exit "$status"
fi
