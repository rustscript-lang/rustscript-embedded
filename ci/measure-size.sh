#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/target/raspi-zero"
ELF="$BUILD_DIR/kernel.elf"
IMG="$BUILD_DIR/kernel.img"

if [[ ! -f "$ELF" ]]; then
  "$ROOT/ci/run-qemu-raspi0.sh" >/dev/null
fi

printf 'elf=%s\n' "$ELF"
arm-none-eabi-size "$ELF"
printf '\nsections:\n'
arm-none-eabi-size -A "$ELF" | sed -n '1,40p'
printf '\nimage_bytes=%s\n' "$(wc -c < "$IMG")"
