#include "DatabaseHandler.h"
#include <mysqlx/xdevapi.h>
#include "../../Constants.h"
#include "../tools/TimeConverter.h"

mysqlx::Session* connection = nullptr;
bool connectedToDatabase = false;

inline std::string getQueryForCreateJobTable(const char tableName[]);
inline void createJobTable(const char tableName[]);
inline void createMeasurementTable(const char tableName[]);
inline bool createDatabaseSchema(const char schemaName[]);
inline void useDatabaseSchema(const char schemaName[]);

bool openDatabaseConnection(const char host[], int port, const char username[], const char password[], const char schemaName[]) {
	try {
		// Create connection and schema
		connection = new mysqlx::Session(mysqlx::SessionOption::HOST, host, mysqlx::SessionOption::PORT, port, mysqlx::SessionOption::USER, username, mysqlx::SessionOption::PWD, password);

		// Create schema
		createDatabaseSchema(schemaName);

		// Use schema
		useDatabaseSchema(schemaName);

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

inline void useDatabaseSchema(const char schemaName[]) {
	connection->sql("USE " + std::string(schemaName)).execute();
}

inline bool createDatabaseSchema(const char schemaName[]) {
	// Check if schema exist
	if (!connection->getSchema(schemaName).existsInDatabase()) {
		// Create schema
		mysqlx::Schema schema = connection->createSchema(schemaName);

		// And then check if the schema exist
		connectedToDatabase = schema.existsInDatabase();
		if (!connectedToDatabase)
			return isConnectedToDatabase();
	}
	else {
		// Seelct schema
		connectedToDatabase = true; // Yes, we have connection
	}

	return isConnectedToDatabase();
}

void dropDatabaseSchema(const char schemaName[]) {
	connection->dropSchema(schemaName);
}

long getDatabaseLatestMeasurementID(const char tableName[]) {
	long measurement_id = 0;
	if (isConnectedToDatabase()) {
		// Select only last row
		std::string query = "SELECT measurement_id FROM " + std::string(tableName) + " ORDER BY measurement_id DESC LIMIT 1";
		mysqlx::SqlResult result = connection->sql(query).execute();
		mysqlx::Row row;
		while (row = result.fetchOne()) {
			measurement_id = row.get(0).get<uint64_t>(); // First row and first column only
		}
	}
	return measurement_id;
}

std::vector<long> getDatabaseMeasurementIDListForCombo(const char tableName[]) {
	std::vector<long> measurementIDList;
	if (isConnectedToDatabase()) {
		// Select only last row
		std::string query = "SELECT DISTINCT measurement_id FROM " + std::string(tableName) + " ORDER BY measurement_id";
		mysqlx::SqlResult result = connection->sql(query).execute();
		mysqlx::Row row = result.fetchOne();
		while (row) {
			int measurement_id = row.get(0).get<uint64_t>(); // First row and first column only
			measurementIDList.push_back(measurement_id);
			row = result.fetchOne();
		}
	}
	return measurementIDList;
}

std::vector<std::string> getDatabaseColumnNames(const char tableName[]) {
	std::vector< std::string> columNames;
	if (isConnectedToDatabase()) {
		// Select only the first row
		std::string query = "SELECT * FROM " + std::string(tableName) + " LIMIT 1";
		mysqlx::SqlResult result = connection->sql(query).execute();
		int columnCount = result.getColumnCount();
		if (result.hasData()) {
			for (long i = 0; i < columnCount; i++) {
				columNames.push_back(result.getColumn(i).getColumnName());
			}
		}
	}
	return columNames;
}

bool uploadDatabaseTable(const char tableName[], std::vector<std::vector<std::string>> table) {
	if (isConnectedToDatabase()) {
		long rowCount = table.size();
		if (rowCount > 0) {
			long columnCount = table.at(0).size();
			for (long i = 1; i < rowCount; i++) {
				std::string query = "INSERT INTO " + std::string(tableName) + " (";
				// Header
				for (long j = 0; j < columnCount - 1; j++) {
					query += table.at(0).at(j) + ",";
				}
				query += table.at(0).at(columnCount - 1) + ") VALUES (";
				// Values
				for (long j = 0; j < columnCount - 1; j++) {
					query += table.at(i).at(j) + ",";
				}
				query += table.at(i).at(columnCount - 1) + ");";
				try {
					connection->sql(query).execute();
				}
				catch (...) {
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

long getIDFromDatabase(const char tableName[], int measurementID, int offset) {
	std::string query = "SELECT id FROM " + std::string(tableName) + " WHERE measurement_id = " + std::to_string(measurementID) + " ORDER BY measurement_id LIMIT " + std::to_string(offset) + ", 1";
	mysqlx::SqlResult result = connection->sql(query).execute();
	long ID = 0;
	if (result.hasData()) {
		mysqlx::Row row;
		int columnCount = result.getColumnCount();
		while (row = result.fetchOne()) {
			for (int i = 0; i < columnCount; i++) {
				switch (row[i].getType()) {
				case mysqlx::common::Value::UINT64:
					ID = row[i].get<uint64_t>();
					break;
				case mysqlx::common::Value::INT64:
					ID = row[i].get<int64_t>();
					break;
				default:
					break;
				}
			}
		}
	}
	return ID;
}

long getRowsLeftFromDatabase(const char tableName[], int measurementID) {
	std::string query = "SELECT COUNT(*) FROM " + std::string(tableName) + " WHERE measurement_id = " + std::to_string(measurementID) + " ORDER BY measurement_id";
	mysqlx::SqlResult result = connection->sql(query).execute();
	long count = 0;
	if (result.hasData()) {
		mysqlx::Row row;
		int columnCount = result.getColumnCount();
		while (row = result.fetchOne()) {
			for (int i = 0; i < columnCount; i++) {
				switch (row[i].getType()) {
				case mysqlx::common::Value::UINT64:
					count = row[i].get<uint64_t>();
					break;
				case mysqlx::common::Value::INT64:
					count = row[i].get<int64_t>();
					break;
				default:
					break;
				}
			}
		}
	}
	return count;
}

void deleteDataFromDatabase(const char measurementTableName[], const char jobTableName[], int measurementID, int offset, int amount) {
	if (isConnectedToDatabase()) {
		// Get first ID
		long firstID = getIDFromDatabase(measurementTableName, measurementID, offset);
		long lastID = getIDFromDatabase(measurementTableName, measurementID, offset + amount - 1);
		
		// Delete between these ID:s
		std::string query = "DELETE FROM " + std::string(measurementTableName) + " WHERE id >= " + std::to_string(firstID) + " AND id <= " + std::to_string(lastID);
		connection->sql(query).execute();

		// Check if there are measurement ID left
		query = "SELECT id FROM " + std::string(measurementTableName) + " WHERE measurement_id = " + std::to_string(measurementID);
		mysqlx::SqlResult result = connection->sql(query).execute();
		if (result.hasData()) {
			// No? Then delete 
			if (!result.fetchOne()) {
				query = "DELETE FROM " + std::string(jobTableName) + " WHERE measurement_id = " + std::to_string(measurementID);
				connection->sql(query).execute();
			}
		}
	}
}

std::vector<std::vector<std::string>> getAllDatabaseValues(const char tableName[], int measurementID, int offset, int amount) {
	std::vector<std::string> values;
	std::vector<std::vector<std::string>> table;
	if (isConnectedToDatabase()) {
		// Select only the first row
		std::string query;
		if (offset == -1 && measurementID == -1) {
			query = "SELECT * FROM " + std::string(tableName) + " ORDER BY measurement_id";
		}
		else {
			if (measurementID >= 0 && offset == -1) {
				query = "SELECT * FROM " + std::string(tableName) + " WHERE measurement_id = " + std::to_string(measurementID) + " ORDER BY measurement_id";
			}
			else {
				query = "SELECT * FROM " + std::string(tableName) + " WHERE measurement_id = " + std::to_string(measurementID) + " ORDER BY measurement_id LIMIT " + std::to_string(offset) + ", " + std::to_string(amount);
			}
		}
		mysqlx::SqlResult result = connection->sql(query).execute();
		int columnCount = result.getColumnCount();
		if (result.hasData()) {
			mysqlx::Row row;
			int year;
			int month;
			int date;
			int hour;
			int minute;
			int second;
			int microsecond;
			mysqlx::bytes data;
			const mysqlx::byte* first;
			char text[50];
			while (row = result.fetchOne()) {
				for (int i = 0; i < columnCount; i++) {
					switch (row[i].getType()) {
					case mysqlx::common::Value::UINT64:
						values.push_back(std::to_string(row[i].get<uint64_t>()));
						break;
					case mysqlx::common::Value::INT64:
						values.push_back(std::to_string(row[i].get<int64_t>()));
						break;
					case mysqlx::common::Value::FLOAT:
						values.push_back(std::to_string(row[i].get<float>()));
						break;
					case mysqlx::common::Value::STRING:
						values.push_back(row[i].get<std::string>());
						break;
					case mysqlx::common::Value::RAW:
						data = row[i].getRawBytes();
						first = data.begin();
						year = (first[1] << 7) | (first[0] & 0x7f);
						month = first[2];
						date = first[3];
						hour = first[4];
						minute = first[5];
						second = first[6];
						microsecond = (first[10] << 21) | (first[9] << 14) | (first[8] << 7) | (first[7] & 0x7f);
						sprintf_s(text, "%i-%i-%i %i:%i:%i.%i", year, month, date, hour, minute, second, microsecond);
						values.push_back(text);
						break;
					default:
						;
						break;
					}
				}
				table.push_back(values);
				values.clear();
			}
		}
	}
	return table;
}

std::vector<std::vector<float>> getMeasurementDatabaseValues(const char tableName[], int measurementID, int offset, int amount) {
	std::vector<float> values;
	std::vector<std::vector<float>> table;
	if (isConnectedToDatabase()) {
		// Select only the first row
		std::string query = "SELECT ";
		for (int i = 0; i < ADC_LENGTH; i++) {
			query += std::string("adc") + std::to_string(i) + std::string(", "); // Analog to digital converter
		}
		for (int i = 0; i < PWM_LENGTH; i++) {
			query += std::string("pwm") + std::to_string(i) + std::string(", "); // Pulse widt modulation
		}
		for (int i = 0; i < DAC_LENGTH; i++) {
			query += std::string("dac") + std::to_string(i) + std::string(", "); // Digital to analog converter
		}
		for (int i = 0; i < DADC_LENGTH; i++) {
			query += std::string("dadc") + std::to_string(i) + std::string(", "); // Differential ADC
		}
		for (int i = 0; i < DI_LENGTH; i++) {
			query += std::string("di") + std::to_string(i) + std::string(", "); // Digital input
		}
		for (int i = 0; i < IC_LENGTH; i++) {
			query += std::string("ic") + std::to_string(i) + std::string(", "); // Input capture
		}
		for (int i = 0; i < E_LENGTH - 1; i++) {
			query += std::string("e") + std::to_string(i) + std::string(", "); // Encoder
		}
		query += "e2 ";
		if (offset == -1) {
			query += "FROM " + std::string(tableName) + " WHERE measurement_id = " + std::to_string(measurementID) + " ORDER BY measurement_id";
		}
		else {
			query += "FROM " + std::string(tableName) + " WHERE measurement_id = " + std::to_string(measurementID) + " ORDER BY measurement_id LIMIT " + std::to_string(offset) + ", " + std::to_string(amount);
		}

		mysqlx::SqlResult result = connection->sql(query).execute();
		int columnCount = result.getColumnCount();
		if (result.hasData()) {
			mysqlx::Row row;
			while (row = result.fetchOne()) {
				for (int i = 0; i < columnCount; i++) {
					switch (row[i].getType()) {
					case mysqlx::common::Value::UINT64:
						values.push_back(row[i].get<uint64_t>());
						break;
					case mysqlx::common::Value::INT64:
						values.push_back(row[i].get<int64_t>());
						break;
					case mysqlx::common::Value::FLOAT:
						values.push_back(row[i].get<float>());
						break;
					}
				}
				table.push_back(values);
				values.clear();
			}
		}
	}
	return table;
}


inline std::string getQueryForCreateMeasurementTable(const char tableName[]) {
	std::string query = "CREATE TABLE IF NOT EXISTS " + std::string(tableName) + " (id BIGINT AUTO_INCREMENT PRIMARY KEY, measurement_id BIGINT, time_stamp VARCHAR(255), sample_time INT,";
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
	for (int i = 0; i < E_LENGTH - 1; i++) {
		query += std::string("e") + std::to_string(i) + std::string(" FLOAT, "); // Encoder
	}
	query += "e2 FLOAT);";
	return query;
}

inline std::string getQueryForCreateJobTable(const char tableName[]) {
	std::string query = "CREATE TABLE IF NOT EXISTS " + std::string(tableName) + " (id BIGINT AUTO_INCREMENT PRIMARY KEY, time_stamp VARCHAR(255), job_name VARCHAR(255), comment VARCHAR(255), measurement_id BIGINT);";
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

