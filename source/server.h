/* $Id: server.h,v 1.4 2002/07/11 21:18:10 slobasso Exp $ */

#ifndef NEDIT_SERVER_H_INCLUDED
#define NEDIT_SERVER_H_INCLUDED

#include <X11/Intrinsic.h>

#define NO_CONNECTION -1
#define COM_OK 1
#define COM_FAILURE 2

void InitServerCommunication(void);
void ServerMainLoop(XtAppContext context);

#endif /* NEDIT_SERVER_H_INCLUDED */
