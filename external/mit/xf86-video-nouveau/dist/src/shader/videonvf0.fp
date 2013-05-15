#ifndef ENVYAS
static uint32_t
NVF0FP_NV12[] = {
	0x00001462,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x80000000,
	0x0000000a,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x0000000f,
	0x00000000,
#include "videonvf0.fpc"
};
#else

interp pass f32 $r2 a[0x7c] 0x0 0x0 0x0
rcp f32 $r2 $r2
interp mul f32 $r0 a[0x80] $r2 0x0 0x0
interp mul f32 $r1 a[0x84] $r2 0x0 0x0
tex t lauto #:#:#:$r4 t2d c[0x0] xy__ $r0:$r1 0x0
tex p lauto #:#:$r0:$r1 t2d c[0x4] xy__ $r0:$r1 0x0
texbar 0x1
mul ftz rn f32 $r5 $r4 c0[0x0]
add ftz rn f32 $r3 $r5 c0[0x4]
add ftz rn f32 $r4 $r5 c0[0x8]
add ftz rn f32 $r5 $r5 c0[0xc]
texbar 0x0
fma ftz rn f32 $r3 $r0 c0[0x10] $r3
fma ftz rn f32 $r4 $r0 c0[0x14] $r4
fma ftz rn f32 $r5 $r0 c0[0x18] $r5
fma ftz rn f32 $r0 $r1 c0[0x1c] $r3
fma ftz rn f32 $r2 $r1 c0[0x24] $r5
fma ftz rn f32 $r1 $r1 c0[0x20] $r4
exit
#endif
