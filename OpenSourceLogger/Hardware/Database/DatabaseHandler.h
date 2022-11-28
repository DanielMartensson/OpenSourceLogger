
#ifndef DatabaseHandler
#define DatabaseHandler

bool openDatabaseConnection(char host[], int port, char username[], char password[], char schemaName[]);
bool closeDatabaseConnection();
bool isConnectedToDatabase();
void dropSchema(char schemaName[]);

#endif // !DatabaseHandler



