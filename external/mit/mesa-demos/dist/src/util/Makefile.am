# Copyright © 2009 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Eric Anholt <eric@anholt.net>

AM_CFLAGS = \
	$(DEMO_CFLAGS)
AM_LDFLAGS = \
	$(DEMO_LIBS)

noinst_LTLIBRARIES = libutil.la

if HAVE_GLUT
AM_CFLAGS += \
	$(GLUT_CFLAGS)
AM_LDFLAGS += \
	$(GLUT_LIBS)
SHADERUTIL_SRC = \
	shaderutil.c \
	shaderutil.h
endif

libutil_la_SOURCES = \
	gl_wrap.h \
	glut_wrap.h \
	readtex.c \
	readtex.h \
	showbuffer.c \
	showbuffer.h \
	trackball.c \
	trackball.h \
	$(SHADERUTIL_SRC)

EXTRA_DIST = \
	README
