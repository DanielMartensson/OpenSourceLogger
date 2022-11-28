#include "DatabaseHandler.h"
#include <mysqlx/xdevapi.h>
#include "../../Constants.h"

mysqlx::Session* connection = nullptr;
bool connectedToDatabase = false;

#define MEASUREMENT_TABLE "measurement_table"
#define JOB_TABLE "job_table"

inline std::string getQueryForCreateJobTable(const char tableName[]);
inline void createJobTable(const char tableName[]);
inline void createMeasurementTable(const char tableName[]);
inline bool createSchema(char schemaName[]);
inline void useSchema(char schemaName[]);

bool openDatabaseConnection(char host[], int port, char username[], char password[], char schemaName[]) {
	try {
		// Create connection and schema
		connection = new mysqlx::Session(mysqlx::SessionOption::HOST, host, mysqlx::SessionOption::PORT, port, mysqlx::SessionOption::USER, username, mysqlx::SessionOption::PWD, password);
		
		// Create schema
		createSchema(schemaName);

		// Use schema
		useSchema(schemaName);

		// Create tables
		createMeasurementTable(MEASUREMENT_TABLE);
		createJobTable(JOB_TABLE);
	}
	catch (...) {}

	// Return status
	return isConnectedToDatabase();
}

bool closeDatabaseConnection() {
	connection->close();
	delete connection;
	connectedToDatabase = false;
	return !isConnectedToDatabase();
}

bool isConnectedToDatabase() {
	return connectedToDatabase;
}

inline void useSchema(char schemaName[]) {
	connection->sql("USE " + std::string(schemaName)).execute();
}

inline bool createSchema(char schemaName[]) {
	// Check if schema exist
	if (!connection->getSchema(schemaName).existsInDatabase()) {
		// Create schema
		mysqlx::Schema schema = connection->createSchema(schemaName);
		
		// And then check if the schema exist
		connectedToDatabase = schema.existsInDatabase();
		if (!connectedToDatabase)
			return isConnectedToDatabase();
	}else {
		// Seelct schema
		connectedToDatabase = true; // Yes, we have connection
	}

	return isConnectedToDatabase();
}

void dropSchema(char schemaName[]) {
	connection->dropSchema(schemaName);
}

inline std::string getQueryForCreateMeasurementTable(const char tableName[]) {
	std::string query = "CREATE TABLE IF NOT EXISTS " + std::string(tableName) + " (id BIGINT AUTO_INCREMENT PRIMARY KEY, measurement_id BIGINT, time_stamp DATETIME(3),"; // DATETIME(3) is milliseconds
	for (int i = 0; i < ADC_LENGTH; i++) {
		query += std::string("adc") + std::to_string(i) + std::string(" FLOAT, "); // Analog to digital converter
	}
	for (int i = 0; i < PWM_LENGTH; i++) {
		query += std::string("pwm") + std::to_string(i) + std::string(" FLOAT, "); // Pulse widt modulation
	}
	for (int i = 0; i < DAC_LENGTH; i++) {
		query += std::string("dac") + std::to_string(i) + std::string(" FLOAT, "); // Digital to analog converter
	}
	for (int i = 0; i < DADC_LENGTH; i++) {
		query += std::string("dadc") + std::to_string(i) + std::string(" FLOAT, "); // Differential ADC
	}
	for (int i = 0; i < DI_LENGTH; i++) {
		query += std::string("di") + std::to_string(i) + std::string(" FLOAT, "); // Digital input
	}
	for (int i = 0; i < IC_LENGTH; i++) {
		query += std::string("ic") + std::to_string(i) + std::string(" FLOAT, "); // Input capture
	}
	for (int i = 0; i < E_LENGTH-1; i++) {
		query += std::string("e") + std::to_string(i) + std::string(" FLOAT, "); // Encoder
	}
	query += "e2 FLOAT);";
	return query;
}

inline std::string getQueryForCreateJobTable(const char tableName[]) {
	std::string query = "CREATE TABLE IF NOT EXISTS " + std::string(tableName) + " (id BIGINT AUTO_INCREMENT PRIMARY KEY, time_stamp DATETIME(6), job_name VARCHAR(256), comment VARCHAR(256), measurement_id BIGINT);";
	return query;
}

inline void createMeasurementTable(const char tableName[]) {
	// Create table if not exist
	connection->sql(getQueryForCreateMeasurementTable(tableName)).execute();
}

inline void createJobTable(const char tableName[]) {
	// Create table if not exist
	connection->sql(getQueryForCreateJobTable(tableName)).execute();
}