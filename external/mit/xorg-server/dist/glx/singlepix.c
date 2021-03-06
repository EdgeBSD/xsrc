/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "glxserver.h"
#include "glxext.h"
#include "singlesize.h"
#include "unpack.h"
#include "indirect_size_get.h"
#include "indirect_dispatch.h"
#include "glapitable.h"
#include "glapi.h"
#include "glthread.h"
#include "dispatch.h"

int __glXDisp_ReadPixels(__GLXclientState *cl, GLbyte *pc)
{
    GLsizei width, height;
    GLenum format, type;
    GLboolean swapBytes, lsbFirst;
    GLint compsize;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];

    REQUEST_FIXED_SIZE(xGLXSingleReq, 28);

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    width = *(GLsizei *)(pc + 8);
    height = *(GLsizei *)(pc + 12);
    format = *(GLenum *)(pc + 16);
    type = *(GLenum *)(pc + 20);
    swapBytes = *(GLboolean *)(pc + 24);
    lsbFirst = *(GLboolean *)(pc + 25);
    compsize = __glReadPixels_size(format,type,width,height);
    if (compsize < 0)
        return BadLength;

    CALL_PixelStorei( GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes) );
    CALL_PixelStorei( GET_DISPATCH(), (GL_PACK_LSB_FIRST, lsbFirst) );
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    CALL_ReadPixels( GET_DISPATCH(), (
		 *(GLint    *)(pc + 0),
		 *(GLint    *)(pc + 4),
		 *(GLsizei  *)(pc + 8),
		 *(GLsizei  *)(pc + 12),
		 *(GLenum   *)(pc + 16),
		 *(GLenum   *)(pc + 20),
		 answer
		 ) );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }
    __GLX_NOTE_FLUSHED_CMDS(cx);
    return Success;
}

int __glXDisp_GetTexImage(__GLXclientState *cl, GLbyte *pc)
{
    GLint level, compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0, height=0, depth=1;

    REQUEST_FIXED_SIZE(xGLXSingleReq, 20);

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    level = *(GLint *)(pc + 4);
    format = *(GLenum *)(pc + 8);
    type = *(GLenum *)(pc + 12);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 16);

    CALL_GetTexLevelParameteriv( GET_DISPATCH(), (target, level, GL_TEXTURE_WIDTH, &width) );
    CALL_GetTexLevelParameteriv( GET_DISPATCH(), (target, level, GL_TEXTURE_HEIGHT, &height) );
    if ( target == GL_TEXTURE_3D) {
	CALL_GetTexLevelParameteriv( GET_DISPATCH(), (target, level, GL_TEXTURE_DEPTH, &depth) );
    }
    /*
     * The three queries above might fail if we're in a state where queries
     * are illegal, but then width, height, and depth would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,level,format,type,width,height,depth);
    if (compsize < 0)
        return BadLength;

    CALL_PixelStorei( GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes) );
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    CALL_GetTexImage( GET_DISPATCH(), (
		  *(GLenum   *)(pc + 0),
		  *(GLint    *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  *(GLenum   *)(pc + 12),
		  answer
		  ) );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	((xGLXGetTexImageReply *)&__glXReply)->width = width;
	((xGLXGetTexImageReply *)&__glXReply)->height = height;
	((xGLXGetTexImageReply *)&__glXReply)->depth = depth;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }
    return Success;
}

int __glXDisp_GetPolygonStipple(__GLXclientState *cl, GLbyte *pc)
{
    GLboolean lsbFirst;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    GLubyte answerBuffer[200];
    char *answer;

    REQUEST_FIXED_SIZE(xGLXSingleReq, 4);

    cx = __glXForceCurrent(cl, __GLX_GET_SINGLE_CONTEXT_TAG(pc), &error);
    if (!cx) {
	return error;
    }

    pc += __GLX_SINGLE_HDR_SIZE;
    lsbFirst = *(GLboolean *)(pc + 0);

    CALL_PixelStorei( GET_DISPATCH(), (GL_PACK_LSB_FIRST, lsbFirst) );
    __GLX_GET_ANSWER_BUFFER(answer,cl,128,1);

    __glXClearErrorOccured();
    CALL_GetPolygonStipple( GET_DISPATCH(), ((GLubyte  *) answer) );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(128);
	__GLX_SEND_HEADER();
	__GLX_SEND_BYTE_ARRAY(128);
    }
    return Success;
}

static int GetSeparableFilter(__GLXclientState *cl, GLbyte *pc, GLXContextTag tag)
{
    GLint compsize, compsize2;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0, height=0;

    cx = __glXForceCurrent(cl, tag, &error);
    if (!cx) {
	return error;
    }

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);

    /* target must be SEPARABLE_2D, however I guess we can let the GL
       barf on this one.... */

    CALL_GetConvolutionParameteriv(GET_DISPATCH(), (target, GL_CONVOLUTION_WIDTH, &width));
    CALL_GetConvolutionParameteriv(GET_DISPATCH(), (target, GL_CONVOLUTION_HEIGHT, &height));
    /*
     * The two queries above might fail if we're in a state where queries
     * are illegal, but then width and height would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,1,1);
    compsize2 = __glGetTexImage_size(target,1,format,type,height,1,1);

    if ((compsize = safe_pad(compsize)) < 0)
        return BadLength;
    if ((compsize2 = safe_pad(compsize2)) < 0)
        return BadLength;

    CALL_PixelStorei(GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes));
    __GLX_GET_ANSWER_BUFFER(answer, cl, safe_add(compsize, compsize2), 1);
    __glXClearErrorOccured();
    CALL_GetSeparableFilter( GET_DISPATCH(), (
		  *(GLenum   *)(pc + 0),
		  *(GLenum   *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  answer,
		  answer + compsize,
		  NULL
		  ) );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize + compsize2);
	((xGLXGetSeparableFilterReply *)&__glXReply)->width = width;
	((xGLXGetSeparableFilterReply *)&__glXReply)->height = height;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize + compsize2);
    }

    return Success;
}

int __glXDisp_GetSeparableFilter(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_SINGLE_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXSingleReq, 16);
    return GetSeparableFilter(cl, pc + __GLX_SINGLE_HDR_SIZE, tag);
}

int __glXDisp_GetSeparableFilterEXT(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_VENDPRIV_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXVendorPrivateReq, 16);
    return GetSeparableFilter(cl, pc + __GLX_VENDPRIV_HDR_SIZE, tag);
}

static int GetConvolutionFilter(__GLXclientState *cl, GLbyte *pc,
				GLXContextTag tag)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0, height=0;

    cx = __glXForceCurrent(cl, tag, &error);
    if (!cx) {
	return error;
    }

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);

    CALL_GetConvolutionParameteriv(GET_DISPATCH(), 
				   (target, GL_CONVOLUTION_WIDTH, &width));
    if (target == GL_CONVOLUTION_1D) {
        height = 1;
    } else {
	CALL_GetConvolutionParameteriv(GET_DISPATCH(),
				       (target, GL_CONVOLUTION_HEIGHT, &height));
    }
    /*
     * The two queries above might fail if we're in a state where queries
     * are illegal, but then width and height would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,height,1);
    if (compsize < 0)
        return BadLength;

    CALL_PixelStorei(GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes));
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    CALL_GetConvolutionFilter( GET_DISPATCH(), (
		  *(GLenum   *)(pc + 0),
		  *(GLenum   *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  answer
		  ) );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	((xGLXGetConvolutionFilterReply *)&__glXReply)->width = width;
	((xGLXGetConvolutionFilterReply *)&__glXReply)->height = height;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDisp_GetConvolutionFilter(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_SINGLE_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXSingleReq, 16);
    return GetConvolutionFilter(cl, pc + __GLX_SINGLE_HDR_SIZE, tag);
}

int __glXDisp_GetConvolutionFilterEXT(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_VENDPRIV_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXVendorPrivateReq, 16);
    return GetConvolutionFilter(cl, pc + __GLX_VENDPRIV_HDR_SIZE, tag);
}

static int GetHistogram(__GLXclientState *cl, GLbyte *pc, GLXContextTag tag)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes, reset;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0;

    cx = __glXForceCurrent(cl, tag, &error);
    if (!cx) {
	return error;
    }

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);
    reset = *(GLboolean *)(pc + 13);

    CALL_GetHistogramParameteriv(GET_DISPATCH(),
				 (target, GL_HISTOGRAM_WIDTH, &width));
    /*
     * The one query above might fail if we're in a state where queries
     * are illegal, but then width would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,1,1);
    if (compsize < 0)
        return BadLength;

    CALL_PixelStorei(GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes));
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    CALL_GetHistogram(GET_DISPATCH(), (target, reset, format, type, answer));

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	((xGLXGetHistogramReply *)&__glXReply)->width = width;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDisp_GetHistogram(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_SINGLE_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXSingleReq, 16);
    return GetHistogram(cl, pc + __GLX_SINGLE_HDR_SIZE, tag);
}

int __glXDisp_GetHistogramEXT(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_VENDPRIV_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXVendorPrivateReq, 16);
    return GetHistogram(cl, pc + __GLX_VENDPRIV_HDR_SIZE, tag);
}

static int GetMinmax(__GLXclientState *cl, GLbyte *pc, GLXContextTag tag)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes, reset;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];

    cx = __glXForceCurrent(cl, tag, &error);
    if (!cx) {
	return error;
    }

    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    target = *(GLenum *)(pc + 0);
    swapBytes = *(GLboolean *)(pc + 12);
    reset = *(GLboolean *)(pc + 13);

    compsize = __glGetTexImage_size(target,1,format,type,2,1,1);
    if (compsize < 0)
        return BadLength;

    CALL_PixelStorei(GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes));
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    CALL_GetMinmax(GET_DISPATCH(), (target, reset, format, type, answer));

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDisp_GetMinmax(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_SINGLE_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXSingleReq, 16);
    return GetMinmax(cl, pc + __GLX_SINGLE_HDR_SIZE, tag);
}

int __glXDisp_GetMinmaxEXT(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_VENDPRIV_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXVendorPrivateReq, 16);
    return GetMinmax(cl, pc + __GLX_VENDPRIV_HDR_SIZE, tag);
}

static int GetColorTable(__GLXclientState *cl, GLbyte *pc, GLXContextTag tag)
{
    GLint compsize;
    GLenum format, type, target;
    GLboolean swapBytes;
    __GLXcontext *cx;
    ClientPtr client = cl->client;
    int error;
    char *answer, answerBuffer[200];
    GLint width=0;

    cx = __glXForceCurrent(cl, tag, &error);
    if (!cx) {
	return error;
    }

    target = *(GLenum *)(pc + 0);
    format = *(GLenum *)(pc + 4);
    type = *(GLenum *)(pc + 8);
    swapBytes = *(GLboolean *)(pc + 12);

    CALL_GetColorTableParameteriv(GET_DISPATCH(), 
				  (target, GL_COLOR_TABLE_WIDTH, &width));
    /*
     * The one query above might fail if we're in a state where queries
     * are illegal, but then width would still be zero anyway.
     */
    compsize = __glGetTexImage_size(target,1,format,type,width,1,1);
    if (compsize < 0)
        return BadLength;

    CALL_PixelStorei(GET_DISPATCH(), (GL_PACK_SWAP_BYTES, swapBytes));
    __GLX_GET_ANSWER_BUFFER(answer,cl,compsize,1);
    __glXClearErrorOccured();
    CALL_GetColorTable( GET_DISPATCH(), (
		  *(GLenum   *)(pc + 0),
		  *(GLenum   *)(pc + 4),
		  *(GLenum   *)(pc + 8),
		  answer
		  ) );

    if (__glXErrorOccured()) {
	__GLX_BEGIN_REPLY(0);
	__GLX_SEND_HEADER();
    } else {
	__GLX_BEGIN_REPLY(compsize);
	((xGLXGetColorTableReply *)&__glXReply)->width = width;
	__GLX_SEND_HEADER();
	__GLX_SEND_VOID_ARRAY(compsize);
    }

    return Success;
}

int __glXDisp_GetColorTable(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_SINGLE_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXSingleReq, 16);
    return GetColorTable(cl, pc + __GLX_SINGLE_HDR_SIZE, tag);
}

int __glXDisp_GetColorTableSGI(__GLXclientState *cl, GLbyte *pc)
{
    const GLXContextTag tag = __GLX_GET_VENDPRIV_CONTEXT_TAG(pc);
    ClientPtr client = cl->client;
    REQUEST_FIXED_SIZE(xGLXVendorPrivateReq, 16);
    return GetColorTable(cl, pc + __GLX_VENDPRIV_HDR_SIZE, tag);
}
