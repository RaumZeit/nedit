/* $Id: server.h,v 1.5 2002/09/11 18:59:49 arnef Exp $ */

#ifndef NEDIT_SERVER_H_INCLUDED
#define NEDIT_SERVER_H_INCLUDED

#include "window.h"

#define NO_CONNECTION -1
#define COM_OK 1
#define COM_FAILURE 2

void InitServerCommunication(void);
void ServerMainLoop(XtAppContext context);
Boolean ServerDispatchEvent(XEvent *event);
void DeleteFileClosedProperty(WindowInfo *window);

#endif /* NEDIT_SERVER_H_INCLUDED */
