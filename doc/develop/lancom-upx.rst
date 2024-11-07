.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2024 J. Neuschäfer

U-Boot as LANCOM UPX file
=========================

The bootloader on LANCOM devices expects firmware in format called UPX.
U-Boot can be packaged as a UPX file. This is useful in the following situations:

- Installing U-Boot (and by extension custom firmware) through the original
  operating system (LCOS)
- Installing U-Boot without replacing the original bootloader, to make it
  easier to go back to LCOS.
