/* SPDX-License-Identifier: MIT */
/* Copyright (C) J. Neuschäfer */

SECTIONS {
	. = 0x01f00000;
	_stack = .;

	.text : {
		_text = .;
		*(.text.crt0);
		*(.text*);
	}

	.rodata : {
		*(.rodata*);
		*(.data.rel.ro*);
	}

	.data : {
		*(.data*);
	}

	_text_size = (. - _start);

	.bss : {
		_bss = .;
		*(.bss*);
	}
	_bss_size = (. - _bss);

}
