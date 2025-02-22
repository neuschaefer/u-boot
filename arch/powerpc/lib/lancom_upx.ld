/* SPDX-License-Identifier: MIT */
/* Copyright (C) J. Neuschäfer */

OUTPUT_ARCH(powerpc)
SECTIONS {
	. = 0x01f00000;
	_stack = .;

	.text : {
		_text = .;
		*(.text*);
	}

	.rodata : {
		*(.rodata*);
		*(.data.rel.ro*);
	}

	.data : {
		*(.data*);
	}

	.bootscript : {
                _bootscript = .;
		*(.bootscript);
	}
	_text_size = (. - _start);

	.bss : {
		_bss = .;
		*(.bss*);
	}
	_bss_size = (. - _bss);

}
ENTRY(_start)
