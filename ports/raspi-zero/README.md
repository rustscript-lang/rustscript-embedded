# Raspberry Pi Zero bare-metal scenario

This port models Raspberry Pi Zero revision 1.2 on BCM2835 / ARM1176JZF-S. It has no Linux ABI and boots through QEMU's `raspi0` machine with a tiny UART firmware.

The firmware runs a frozen RustScript-flavored bytecode form of `programs/blinky.rss`. It is deliberately small and has no JIT path. The host examples still show the normal `pd-vm` API; this port shows how a constrained board can carry a frozen script payload and a tiny interpreter loop.

```bash
ci/run-qemu-raspi0.sh
ci/measure-size.sh
```
