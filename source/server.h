#define NO_CONNECTION -1
#define COM_OK 1
#define COM_FAILURE 2

void InitServerCommunication(void);
void ServerMainLoop(XtAppContext context);
char *CreateServerPropName(char *propType);
