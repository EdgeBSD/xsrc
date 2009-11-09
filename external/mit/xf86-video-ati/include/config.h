/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

#include "xorg-server.h"

/* Use Damage extension */
#define DAMAGE 1

/* Use linux pragma pack */
/* #undef FGL_LINUX */

/* Define to 1 if you have the <byteswap.h> header file. */
/* #undef HAVE_BYTESWAP_H */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* have new FreeShadow API */
/* #undef HAVE_FREE_SHADOW */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Have pci_device_enable prototype */
/* #undef HAVE_PCI_DEVICE_ENABLE */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Have xf86_crtc_clip_video_helper prototype */
/* #undef HAVE_XF86CRTCCLIPVIDEOHELPER */

/* Have xf86ModeBandwidth prototype */
/* #undef HAVE_XF86MODEBANDWIDTH */

/* Have xf86XVFillKeyHelperDrawable prototype */
#define HAVE_XV_DRAWABLE_HELPER 1

/* Name of package */
#define PACKAGE "xf86-video-ati"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://bugs.freedesktop.org/enter_bug.cgi?product=xorg"

/* Define to the full name of this package. */
#define PACKAGE_NAME "xf86-video-ati"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "xf86-video-ati 6.12.2"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "xf86-video-ati"

/* Define to the version of this package. */
#define PACKAGE_VERSION "6.12.2"

/* Major version of this package */
#define PACKAGE_VERSION_MAJOR 6

/* Minor version of this package */
#define PACKAGE_VERSION_MINOR 12

/* Patch version of this package */
#define PACKAGE_VERSION_PATCHLEVEL 2

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Build support for Exa */
#define USE_EXA 1

/* Define to use byteswap macros from <sys/endian.h> */
#define USE_SYS_ENDIAN_H 1

/* Build support for XAA */
#define USE_XAA 1

/* Version number of package */
#define VERSION "6.12.2"

/* Enable DRI driver support */
#define XF86DRI 1

/* Enable developmental DRI driver support */
#define XF86DRI_DEVEL 1

/* X server has built-in mode code */
#define XMODES 1

/* Define to 16-bit byteswap macro */
#define bswap_16 bswap16

/* Define to 32-bit byteswap macro */
#define bswap_32 bswap32

/* Define to 64-bit byteswap macro */
#define bswap_64 bswap64