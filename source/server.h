/* $Id: server.h,v 1.3 2001/08/25 15:58:54 amai Exp $ */
#define NO_CONNECTION -1
#define COM_OK 1
#define COM_FAILURE 2

void InitServerCommunication(void);
void ServerMainLoop(XtAppContext context);
