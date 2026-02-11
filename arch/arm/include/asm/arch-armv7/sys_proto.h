/*
 *(C) Copyright 2008
 *Texas Instruments, <www.ti.com>
 *
 *Richard Woodruff <r-woodru...@ti.com>
 *Syed Mohammed Khasim <kha...@ti.com>
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation; either version 2 of
 *the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *MA 02111-1307 USA
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

void sr32(void *, u32, u32, u32);
u32 wait_on_value(u32, u32, void *, u32);
void sdelay(unsigned long);

#endif

