#ifndef GEN_BLITTER_XML
#define GEN_BLITTER_XML

/* Autogenerated file, DO NOT EDIT manually!

This file was generated by the rules-ng-ng headergen tool in this git repository:
https://github.com/olvaffe/envytools/
git clone https://github.com/olvaffe/envytools.git

Copyright (C) 2014 by the following authors:
- Chia-I Wu <olvaffe@gmail.com> (olv)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#define GEN6_BLITTER_TYPE__MASK					0xe0000000
#define GEN6_BLITTER_TYPE__SHIFT				29
#define GEN6_BLITTER_TYPE_BLITTER				(0x2 << 29)
#define GEN6_BLITTER_OPCODE__MASK				0x1fc00000
#define GEN6_BLITTER_OPCODE__SHIFT				22
#define GEN6_BLITTER_OPCODE_COLOR_BLT				(0x40 << 22)
#define GEN6_BLITTER_OPCODE_SRC_COPY_BLT			(0x43 << 22)
#define GEN6_BLITTER_OPCODE_XY_COLOR_BLT			(0x50 << 22)
#define GEN6_BLITTER_OPCODE_XY_SRC_COPY_BLT			(0x53 << 22)
#define GEN6_BLITTER_BR00_WRITE_A				(0x1 << 21)
#define GEN6_BLITTER_BR00_WRITE_RGB				(0x1 << 20)
#define GEN6_BLITTER_BR00_SRC_TILED				(0x1 << 15)
#define GEN6_BLITTER_BR00_DST_TILED				(0x1 << 11)
#define GEN6_BLITTER_LENGTH__MASK				0x0000003f
#define GEN6_BLITTER_LENGTH__SHIFT				0
#define GEN6_BLITTER_BR13_CLIP_ENABLE				(0x1 << 30)
#define GEN6_BLITTER_BR13_DIR_RTL				(0x1 << 30)
#define GEN6_BLITTER_BR13_FORMAT__MASK				0x03000000
#define GEN6_BLITTER_BR13_FORMAT__SHIFT				24
#define GEN6_BLITTER_BR13_FORMAT_8				(0x0 << 24)
#define GEN6_BLITTER_BR13_FORMAT_565				(0x1 << 24)
#define GEN6_BLITTER_BR13_FORMAT_1555				(0x2 << 24)
#define GEN6_BLITTER_BR13_FORMAT_8888				(0x3 << 24)
#define GEN6_BLITTER_BR13_ROP__MASK				0x00ff0000
#define GEN6_BLITTER_BR13_ROP__SHIFT				16
#define GEN6_BLITTER_BR13_ROP_SRCCOPY				(0xcc << 16)
#define GEN6_BLITTER_BR13_ROP_PATCOPY				(0xf0 << 16)
#define GEN6_BLITTER_BR13_DST_PITCH__MASK			0x0000ffff
#define GEN6_BLITTER_BR13_DST_PITCH__SHIFT			0
#define GEN6_BLITTER_BR11_SRC_PITCH__MASK			0x0000ffff
#define GEN6_BLITTER_BR11_SRC_PITCH__SHIFT			0
#define GEN6_BLITTER_BR14_DST_HEIGHT__MASK			0xffff0000
#define GEN6_BLITTER_BR14_DST_HEIGHT__SHIFT			16
#define GEN6_BLITTER_BR14_DST_WIDTH__MASK			0x0000ffff
#define GEN6_BLITTER_BR14_DST_WIDTH__SHIFT			0
#define GEN6_BLITTER_BR22_DST_Y1__MASK				0xffff0000
#define GEN6_BLITTER_BR22_DST_Y1__SHIFT				16
#define GEN6_BLITTER_BR22_DST_X1__MASK				0x0000ffff
#define GEN6_BLITTER_BR22_DST_X1__SHIFT				0
#define GEN6_BLITTER_BR23_DST_Y2__MASK				0xffff0000
#define GEN6_BLITTER_BR23_DST_Y2__SHIFT				16
#define GEN6_BLITTER_BR23_DST_X2__MASK				0x0000ffff
#define GEN6_BLITTER_BR23_DST_X2__SHIFT				0
#define GEN6_BLITTER_BR26_SRC_Y1__MASK				0xffff0000
#define GEN6_BLITTER_BR26_SRC_Y1__SHIFT				16
#define GEN6_BLITTER_BR26_SRC_X1__MASK				0x0000ffff
#define GEN6_BLITTER_BR26_SRC_X1__SHIFT				0
#define GEN6_COLOR_BLT__SIZE					5






#define GEN6_SRC_COPY_BLT__SIZE					6







#define GEN6_XY_COLOR_BLT__SIZE					6







#define GEN6_XY_SRC_COPY_BLT__SIZE				8










#endif /* GEN_BLITTER_XML */
