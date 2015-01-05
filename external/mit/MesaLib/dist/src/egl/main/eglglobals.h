/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
 * Copyright 2010-2011 LunarG, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#ifndef EGLGLOBALS_INCLUDED
#define EGLGLOBALS_INCLUDED

#include <stdbool.h>

#include "egltypedefs.h"
#include "eglmutex.h"


/**
 * Global library data
 */
struct _egl_global
{
   _EGLMutex *Mutex;

   /* the list of all displays */
   _EGLDisplay *DisplayList;

   EGLint NumAtExitCalls;
   void (*AtExitCalls[10])(void);

   struct _egl_client_extensions {
      bool EXT_client_extensions;
      bool EXT_platform_base;
      bool EXT_platform_x11;
      bool EXT_platform_wayland;
      bool MESA_platform_gbm;
   } ClientExtensions;

   const char *ClientExtensionString;
};


extern struct _egl_global _eglGlobal;


extern void
_eglAddAtExitCall(void (*func)(void));


#endif /* EGLGLOBALS_INCLUDED */
