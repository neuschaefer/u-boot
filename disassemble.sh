#!/bin/sh
arm-linux-objdump -h -S -l -m armv4 --show-raw-insn -EL u-boot > u-boot.dis

