/**
 *
 * $Id: BubbleButtonP.h,v 1.4 2005/11/30 17:48:04 tringali Exp $
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
#ifndef _BUBBLEBUTTONP_H
#define _BUBBLEBUTTONP_H

#include "BubbleButton.h"
#include <Xm/PushBP.h>

typedef struct {
	XtIntervalId Timer;
	int Delay;
	Widget BubbleLabel;
	XmString BubbleString;
	Boolean show_bubble;
	XmString MouseOverString;
	Pixmap MouseOverPixmap;
	XtIntervalId DurationTimer;
	int Duration;
	Boolean Swapped;
	Widget slider;
	Boolean slidingBubble;
	Boolean autoParkBubble;
} XltBubbleButtonPart;

#define XltBubbleButtonField(w,f,t) XmField(w, XltBubbleButton_offsets, XltBubbleButton, f, t)

#define XltBubbleButtonIndex (XmPushButtonIndex + 1)

extern XmOffsetPtr XltBubbleButton_offsets;

#define BubbleButton_Timer(w) XltBubbleButtonField(w, Timer, XtIntervalId)
#define BubbleButton_Delay(w) XltBubbleButtonField(w, Delay, int)
#define BubbleButton_Label(w) XltBubbleButtonField(w, BubbleLabel, Widget)
#define BubbleButton_BubbleString(w) XltBubbleButtonField(w, BubbleString, XmString)
#define BubbleButton_ShowBubble(w) XltBubbleButtonField(w, show_bubble, Boolean)
#define BubbleButton_MouseOverString(w) XltBubbleButtonField(w, MouseOverString, XmString)
#define BubbleButton_MouseOverPixmap(w) XltBubbleButtonField(w, MouseOverPixmap, Pixmap)
#define BubbleButton_DurationTimer(w) XltBubbleButtonField(w, DurationTimer, XtIntervalId)
#define BubbleButton_Duration(w) XltBubbleButtonField(w, Duration, int)
#define BubbleButton_Swapped(w) XltBubbleButtonField(w, Swapped, Boolean)
#define BubbleButton_Slider(w) XltBubbleButtonField(w, slider, Widget)
#define BubbleButton_SlidingBubble(w) XltBubbleButtonField(w, slidingBubble, Boolean)
#define BubbleButton_AutoParkBubble(w) XltBubbleButtonField(w, autoParkBubble, Boolean)

#define BubbleButtonClass_LeaveTime(w) (((XltBubbleButtonWidgetClass)XtClass(w))->bubble_button_class.leave_time)

typedef struct _XltBubbleButtonRec {
	CorePart core;
	XmPrimitivePart primitive;
	XmLabelPart label;
	XmPushButtonPart pushbutton;
	XltBubbleButtonPart bubble_button;
} XltBubbleButtonRec;

typedef struct {
	Time leave_time;
	XtPointer extension;
} XltBubbleButtonClassPart;

typedef struct _XltBubbleButtonClassRec {
	CoreClassPart core_class;
	XmPrimitiveClassPart primitive_class;
	XmLabelClassPart label_class;
	XmPushButtonClassPart pushbutton_class;
	XltBubbleButtonClassPart bubble_button_class;
} XltBubbleButtonClassRec;

extern XltBubbleButtonClassRec xrwsBubbleButtonClassRec;
#endif
