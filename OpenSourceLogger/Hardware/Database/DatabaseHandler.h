
#include <string>
#include <vector>

#ifndef DatabaseHandler
#define DatabaseHandler

#define MEASUREMENT_TABLE "measurement_table"
#define JOB_TABLE "job_table"

bool openDatabaseConnection(char host[], int port, char username[], char password[], char schemaName[]);
bool closeDatabaseConnection();
bool isConnectedToDatabase();
void dropDatabaseSchema(char schemaName[]);
long getDatabaseLatestMeasurementID(const char tableName[]);
std::vector<long> getDatabaseMeasurementIDListForCombo(const char tableName[]);
std::vector<std::string> getDatabaseColumnNames(const char tableName[]);
std::vector<std::vector<std::string>> getAllDatabaseValues(const char tableName[], int measurementID = -1, int offset = -1, int amount = -1);
std::vector<std::vector<float>> getMeasurementDatabaseValues(const char tableName[], int measurementID, int offset, int amount);
bool uploadDatabaseTable(const char tableName[], std::vector<std::vector<std::string>> table);
void deleteDataFromDatabase(const char measurementTableName[], const char jobTableName[], int measurementID, int offset, int amount);
long getIDFromDatabase(const char tableName[], int measurementID, int offset);
long getRowsLeftFromDatabase(const char tableName[], int measurementID);
#endif // !DatabaseHandler



