#include "UploadMeasurementToDatabaseDialog.h"
#include "../FileDialog/FileDialog.h"
#include "imgui.h"
#include "../../../Hardware/Database/DatabaseHandler.h"
#include "../../../Hardware/Tools/TimeConverter.h"
#include "../../../Constants.h"
#include <fstream>
#include <sstream>

inline void createTable(const char tableID[], std::vector<std::vector<std::string>> table, int offset);
inline void loadJobTable();
long measurement_id = 0;
std::vector<std::vector<std::string>> measurementTableValues;
std::vector<std::vector<std::string>> jobTableValues;

void showUploadMeasurementToDatabaseDialog(bool* uploadMeasurementToDatabase) {
	// Check if you are connected to database
	ImGui::SetNextWindowSize(ImVec2(800.0f, 500.0f));
	ImGui::Begin("Upload measurements", uploadMeasurementToDatabase, ImGuiWindowFlags_NoResize);
	if (isConnectedToDatabase()) {
		static char file_folder_path[1000] = { 0 };
		ImGui::InputText("File path", file_folder_path, sizeof(file_folder_path), ImGuiInputTextFlags_ReadOnly);
		static char jobName[100] = { 0 };
		static char comment[100] = { 0 };
		ImGui::InputText("Job name:", jobName, sizeof(jobName));
		ImGui::InputText("Comment:", comment, sizeof(comment));
		static bool file_dialog_open = false;
		static bool isFileLoaded = false;
		if (ImGui::Button("Select measurement file")) {
			file_dialog_open = true;
			isFileLoaded = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Update job table")) {
			loadJobTable();
		}
		if (isFileLoaded) {
			ImGui::SameLine();
			if (ImGui::Button("Upload to database")) {
				// Upload measurements
				if (uploadDatabaseTable(MEASUREMENT_TABLE, measurementTableValues)) {
					// Upload job - Header
					std::vector<std::vector<std::string>> newJobRow;
					newJobRow.push_back(getDatabaseColumnNames(JOB_TABLE));

					// Upload job - data
					std::vector<std::string> row;
					row.push_back("0"); // ID
					row.push_back(currentISO8601Time()); // Time stamp
					row.push_back("'" + std::string(jobName) + "'"); // Job name
					row.push_back("'" + std::string(comment) + "'"); // Comment of the measurement
					row.push_back(std::to_string(measurement_id)); // Measurement ID
					newJobRow.push_back(row);
					uploadDatabaseTable(JOB_TABLE, newJobRow);
					loadJobTable();
					isFileLoaded = false;
				}
			}
		}

		// When file is selected, then this if-satement runs
		if (showFileDialog(&file_dialog_open, file_folder_path, sizeof(file_folder_path), FileDialogType::OpenFile)) {
			// Clear
			measurementTableValues.clear();

			// Read the measurement table column names
			std::vector<std::string> columnNamesMeasurement = getDatabaseColumnNames(MEASUREMENT_TABLE);

			// Add
			measurementTableValues.push_back(columnNamesMeasurement);

			// Get measurement ID
			measurement_id = getDatabaseLatestMeasurementID(JOB_TABLE) + 1;

			// Create the measurement table values from file
			std::ifstream file;
			file.open(file_folder_path);
			if (file.is_open()) {
				std::string line;
				while (std::getline(file, line)) {
					// Split and add to the data
					std::vector<std::string> data;
					std::string slittedLine;
					std::stringstream splitted(line);
					data.push_back("0"); // ID
					data.push_back(std::to_string(measurement_id)); // Measurement ID
					while (std::getline(splitted, slittedLine, ',')) {
						data.push_back(slittedLine); // time_stamp, sample_time, adc0, adc1, adc3, ...
					}
					// Now add data to tableValues
					int correctColumnLength = 4 + ALL_MEASUREMENT_LENGTH; // 4 = ID, measurement I, time_stamp and sample_time
					int columnLength = data.size();
					if (columnLength != correctColumnLength) {
						sprintf(file_folder_path, "");
						break;
					}
					measurementTableValues.push_back(data);
					// Is file loaded
					isFileLoaded = true;
				}
				file.close();
			}

			// Read the measurement table column names
			loadJobTable();
			file_dialog_open = false;
		}

		if (ImGui::CollapsingHeader("Job table")) {
			int rowSize = jobTableValues.size();
			static int offset = 0;
			if (ImGui::InputInt("Offset", &offset)) {
				if (offset <= -1) {
					offset++;
				}
				if (offset >= rowSize) {
					offset = rowSize - 1;
				}
			}
			createTable("jobTableValues", jobTableValues, offset);
		}
		if (ImGui::CollapsingHeader("Measurement table")) {
			int rowSize = measurementTableValues.size();
			static int offset = 0;
			if (ImGui::InputInt("Offset", &offset)) {
				if (offset <= -1) {
					offset++;
				}
				if (offset >= rowSize) {
					offset = rowSize - 1;
				}
			}
			createTable("measurementTableValues", measurementTableValues, offset);
		}
	}
	else {
		ImGui::Text("You need to be connected to the database");
	}

	ImGui::End();
}

inline void createTable(const char tableID[], std::vector<std::vector<std::string>> table, int offset) {
	long rowSize = table.size();
	if (rowSize > 0) {
		long columnSize = table.at(0).size();
		if (ImGui::BeginTable(tableID, columnSize, ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_Resizable)) {
			// Create the header
			for (long i = 0; i < columnSize; i++) {
				ImGui::TableSetupColumn(table.at(0).at(i).c_str());
			}
			ImGui::TableHeadersRow();

			// Fill the rows
			long rowIndex = offset + 1; // +1 due to the header
			while (rowIndex < rowSize && rowIndex < 20 + offset) {
				ImGui::TableNextRow();
				for (long j = 0; j < columnSize; j++) {
					ImGui::TableSetColumnIndex(j);
					ImGui::Text(table.at(rowIndex).at(j).c_str());
				}
				rowIndex++;
			}
			ImGui::EndTable();
		}
	}
}

inline void loadJobTable() {
	jobTableValues.clear();
	jobTableValues = getAllDatabaseValues(JOB_TABLE);
	std::vector<std::string> columnNames = getDatabaseColumnNames(JOB_TABLE);
	jobTableValues.insert(jobTableValues.begin(), columnNames); // On top
}