/**
 *
 * $Id: SlideCP.h,v 1.3 2005/11/30 17:48:04 tringali Exp $
 *
 * Copyright (C) 1996 Free Software Foundation, Inc.
 * Copyright © 1999-2001 by the LessTif developers.
 *
 * This file is part of the GNU LessTif Extension Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 **/

#ifndef _SLIDECP_H
#define _SLIDECP_H

#include <X11/IntrinsicP.h>
#include <X11/ObjectP.h>
#include <Xm/XmP.h>
#include "SlideC.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifndef XmUNSPECIFIED
#define XmUNSPECIFIED  (~0)
#endif

#ifndef XmUNSPECIFIED_POSITION
#define XmUNSPECIFIED_POSITION (-1)
#endif

typedef struct {
    XtPointer extension;
} XltSlideContextClassPart;

typedef struct _XltSlideContextClassRec {
	ObjectClassPart object_class;
	XltSlideContextClassPart slide_class;
} XltSlideContextClassRec;

extern XltSlideContextClassRec xltSlideContextClassRec;

typedef struct _XmSlideContextPart {
	XtIntervalId id;
	XtCallbackList slideFinishCallback;
	XtCallbackList slideMotionCallback;
	Widget slide_widget;
	unsigned long interval;
	Dimension dest_width;
	Dimension dest_height;
	Position dest_x;
	Position dest_y;
} XltSlideContextPart;

typedef struct _XltSlideContextRec {
	ObjectPart object;
	XltSlideContextPart slide;
} XltSlideContextRec;

#define XltSlideContextField(w,f,t) XmField(w, XltSlideContext_offsets, XltSlideContext, f, t)

#define XltSlideContextIndex (XmObjectIndex + 1)

extern XmOffsetPtr XltSlideContext_offsets;

#define Slide_Id(w) XltSlideContextField(w, id, XtIntervalId)
#define Slide_Widget(w) XltSlideContextField(w, slide_widget, Widget)
#define Slide_Interval(w) XltSlideContextField(w, interval, unsigned long)
#define Slide_DestWidth(w) XltSlideContextField(w, dest_width, Dimension)
#define Slide_DestHeight(w) XltSlideContextField(w, dest_height, Dimension)
#define Slide_DestX(w) XltSlideContextField(w, dest_x, Position)
#define Slide_DestY(w) XltSlideContextField(w, dest_y, Position)
#define Slide_FinishCallback(w) XltSlideContextField(w, slideFinishCallback, XtCallbackList)
#define Slide_MotionCallback(w) XltSlideContextField(w, slideMotionCallback, XtCallbackList)

#ifdef __cplusplus
}
#endif

#endif /* ifndef _SLIDECP_H */
