/* $Id: server.h,v 1.2 2001/02/26 23:38:03 edg Exp $ */
#define NO_CONNECTION -1
#define COM_OK 1
#define COM_FAILURE 2

void InitServerCommunication(void);
void ServerMainLoop(XtAppContext context);
char *CreateServerPropName(char *propType);
