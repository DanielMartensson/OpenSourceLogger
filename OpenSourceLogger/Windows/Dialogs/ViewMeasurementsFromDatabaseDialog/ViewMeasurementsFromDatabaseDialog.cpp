#include "ViewMeasurementsFromDatabaseDialog.h"
#include "imgui.h"
#include "../../../Hardware/Database/DatabaseHandler.h"
#include "../../../Constants.h"
#include <algorithm>
#include <fstream>
#include "../FileDialog/FileDialog.h"

inline void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray);
inline void loadMeasurements(int offset, int amount, int measurementID);
inline void loadAllDatabaseValues(int offset, int amount, int measurementID);
inline void createMeasurementPlots(std::vector<float> lineChart[], const char pheferialName[], bool enable[], int dimensionOfVector);
inline void fillMeasurementsVector(std::vector<float> lineChart[], const int dimensionOfVector, const int columnInDatabase);
inline int createJobTable(const char tableID[], std::vector<std::vector<std::string>> table, int offset);
inline void updateJobTable();
std::vector<std::vector<float>> measurementTableValues;
std::vector<std::vector<std::string>> databaseValues;
std::vector<std::vector<std::string>> jobTableValues2;
int selectedRowIndex;

void showViewMeasurementFromDatabaseDialog(bool* measurementsFromDatabase) {
	ImGui::SetNextWindowSize(ImVec2(850.0f, 850.0f));
	ImGui::Begin("View measurements from database", measurementsFromDatabase, ImGuiWindowFlags_NoResize);
	if (isConnectedToDatabase()) {
		// Enable fields
		static bool enableADC[ADC_LENGTH];
		static bool enableDADC[DADC_LENGTH];
		static bool enableDAC[DAC_LENGTH];
		static bool enablePWM[PWM_LENGTH];
		static bool enableDI[DI_LENGTH];
		static bool enableIC[IC_LENGTH];
		static bool enableE[E_LENGTH];

		// Data plot fields
		static std::vector<float> ADC[ADC_LENGTH];
		static std::vector<float> DADC[DADC_LENGTH];
		static std::vector<float> DAC[DAC_LENGTH];
		static std::vector<float> PWM[PWM_LENGTH];
		static std::vector<float> DI[DI_LENGTH];
		static std::vector<float> IC[IC_LENGTH];
		static std::vector<float> E[E_LENGTH];

		// Create checkboxes
		ImGui::BeginChild("groupBoxEnable", ImVec2(0, ImGui::GetFontSize() * 15.0f), true);
		ImGui::Text("Enable measurements:");
		createCheckBoxes("ADC", enableADC, ADC_LENGTH);
		createCheckBoxes("DADC", enableDADC, DADC_LENGTH);
		createCheckBoxes("DAC", enableDAC, DAC_LENGTH);
		createCheckBoxes("PWM", enablePWM, PWM_LENGTH);
		createCheckBoxes("DI", enableDI, DI_LENGTH);
		createCheckBoxes("IC", enableIC, IC_LENGTH);
		createCheckBoxes("E", enableE, E_LENGTH);
		ImGui::EndChild();

		// Sample settings
		if (ImGui::Button("Update job table")) {
			updateJobTable();
		}

		long measurementID = 0;
		int samplesLeft = 0;
		int rowSize = jobTableValues2.size();
		static int offset = 0;
		if (ImGui::InputInt("Offset job table", &offset)) {
			if (offset <= -1) {
				offset++;
			}
			if (offset >= rowSize) {
				offset = rowSize - 1;
			}
		}

		// Create table and get selected rowIndex
		int selectedRowIndex = createJobTable("jobTableValuesView", jobTableValues2, offset);
		if (selectedRowIndex > 0) {
			// Get the rowsLeft column
			long rowSize = jobTableValues2.at(selectedRowIndex).size();
			samplesLeft = std::stoi(jobTableValues2.at(selectedRowIndex).at(rowSize - 1));
			measurementID = std::stoi(jobTableValues2.at(selectedRowIndex).at(rowSize - 2));
		}

		// Get data
		static int offsetPlot = 0;
		static int samplesPlot = 0;
		ImGui::InputInt("Offset plot:", &offsetPlot);
		if (offsetPlot > samplesLeft) {
			offsetPlot = samplesLeft;
		}
		if (offsetPlot + samplesPlot > samplesLeft) {
			samplesPlot = samplesLeft - offsetPlot;
		}
		if (offsetPlot <= -1) {
			offsetPlot++;
		}
		ImGui::InputInt("Samples plot:", &samplesPlot);
		if (offsetPlot + samplesPlot > samplesLeft) {
			samplesPlot = samplesLeft - offsetPlot;
		}
		if (samplesPlot <= 0) {
			samplesPlot++;
		}
		if (ImGui::Button("Plot data") && jobTableValues2.size() > 0) {
			loadMeasurements(offsetPlot, samplesPlot, measurementID);
			loadAllDatabaseValues(offsetPlot, samplesPlot, measurementID);
			fillMeasurementsVector(ADC, ADC_LENGTH, 0);
			fillMeasurementsVector(PWM, PWM_LENGTH, ADC_LENGTH);
			fillMeasurementsVector(DAC, DAC_LENGTH, PWM_LENGTH + ADC_LENGTH);
			fillMeasurementsVector(DADC, DADC_LENGTH, DAC_LENGTH + PWM_LENGTH + ADC_LENGTH);
			fillMeasurementsVector(DI, DI_LENGTH, DADC_LENGTH + DAC_LENGTH + PWM_LENGTH + ADC_LENGTH);
			fillMeasurementsVector(IC, IC_LENGTH, DI_LENGTH + DADC_LENGTH + DAC_LENGTH + PWM_LENGTH + ADC_LENGTH);
			fillMeasurementsVector(E, E_LENGTH, IC_LENGTH + DI_LENGTH + DADC_LENGTH + DAC_LENGTH + PWM_LENGTH + ADC_LENGTH);
		}

		// Save file
		ImGui::SameLine();
		static bool openFileDialog = false;
		static char file_folder_path[1000] = { 0 };
		if (ImGui::Button("Download data")) {
			openFileDialog = true;
		}
		if (showFileDialog(&openFileDialog, file_folder_path, sizeof(file_folder_path), FileDialogType::OpenFile)) {
			openFileDialog = false;
			std::ofstream logFile;
			logFile.open(file_folder_path);
			if (logFile.is_open()) {
				int rowSize = databaseValues.size();
				if (rowSize > 0) {
					int columnSize = databaseValues.at(0).size();
					for (int i = 0; i < rowSize; i++) {
						for (int j = 0; j < columnSize; j++) {
							logFile << databaseValues.at(i).at(j) + ",";
						}
						logFile << "\n";
					}
				}
			}
		}

		// Delete button
		ImGui::SameLine();
		if (ImGui::Button("Delete data") && jobTableValues2.size() > 0) {
			deleteDataFromDatabase(MEASUREMENT_TABLE, JOB_TABLE, measurementID, offsetPlot, samplesPlot);
			updateJobTable();
			selectedRowIndex = 0;
		}

		// Show plots
		createMeasurementPlots(ADC, "ADC", enableADC, ADC_LENGTH);
		createMeasurementPlots(DADC, "DADC", enableDADC, DADC_LENGTH);
		createMeasurementPlots(DAC, "DAC", enableDAC, DAC_LENGTH);
		createMeasurementPlots(PWM, "PWM", enablePWM, PWM_LENGTH);
		createMeasurementPlots(DI, "DI", enableDI, DI_LENGTH);
		createMeasurementPlots(IC, "IC", enableIC, IC_LENGTH);
		createMeasurementPlots(E, "E", enableE, E_LENGTH);
	}
	else {
		ImGui::Text("You need to be connected to the database");
	}
	ImGui::End();
}

inline void createCheckBoxes(const char pheferialName[], bool enable[], const int lengthOfArray) {
	for (int i = 0; i < lengthOfArray; i++) {
		std::string text = std::string(pheferialName) + std::to_string(i);
		ImGui::Checkbox(text.data(), &enable[i]);
		ImGui::SameLine();
	}
	ImGui::NewLine();
}

inline void loadMeasurements(int offset, int amount, int measurementID) {
	measurementTableValues.clear();
	measurementTableValues = getMeasurementDatabaseValues(MEASUREMENT_TABLE, measurementID, offset, amount);
}

inline void loadAllDatabaseValues(int offset, int amount, int measurementID) {
	databaseValues.clear();
	databaseValues = getAllDatabaseValues(MEASUREMENT_TABLE, measurementID, offset, amount);
	std::vector<std::string> columnNames = getDatabaseColumnNames(MEASUREMENT_TABLE);
	databaseValues.insert(databaseValues.begin(), columnNames); // On top
}

inline void fillMeasurementsVector(std::vector<float> lineChart[], const int dimensionOfVector, const int columnInDatabase) {
	int rowSize = measurementTableValues.size();
	for (int i = 0; i < dimensionOfVector; i++) {
		lineChart[i].clear();
		for (int j = 1; j < rowSize; j++) {
			float value = measurementTableValues.at(j).at(i + columnInDatabase);
			lineChart[i].push_back(value);
		}
	}
}

inline void createMeasurementPlots(std::vector<float> lineChart[], const char pheferialName[], bool enable[], int dimensionOfVector) {
	for (int i = 0; i < dimensionOfVector; i++) {
		// Show plot if enabled
		if (enable[i] && lineChart[i].size() > 0) {
			// Create the plot - Variable size
			std::string text = std::string(pheferialName) + std::to_string(i) + " value:" + std::to_string(lineChart[i].back());
			float minPlotSize = *min_element(lineChart[i].begin(), lineChart[i].end());
			float maxPlotSize = *max_element(lineChart[i].begin(), lineChart[i].end());
			ImGui::PlotLines(text.data(), lineChart[i].data(), lineChart[i].size(), 0, "", minPlotSize, maxPlotSize, ImVec2(0, 100.0f));
		}
	}
}

inline int createJobTable(const char tableID[], std::vector<std::vector<std::string>> table, int offset) {
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
			static bool isSelected = false;
			while (rowIndex < rowSize && rowIndex < 20 + offset) {
				ImGui::TableNextRow();
				for (long j = 0; j < columnSize; j++) {
					ImGui::TableSetColumnIndex(j);
					isSelected = selectedRowIndex == rowIndex ? true : false;
					if (ImGui::Selectable(table.at(rowIndex).at(j).c_str(), &isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
						selectedRowIndex = rowIndex;
					}
				}
				rowIndex++;
			}
			ImGui::EndTable();
		}
	}
	else {
		selectedRowIndex = 0;
	}
	return selectedRowIndex;
}

inline void updateJobTable() {
	// Get data from Job table
	jobTableValues2.clear();
	std::vector<long> measurementIDList = getDatabaseMeasurementIDListForCombo(JOB_TABLE);
	if (measurementIDList.size() == 0)
		return;
	jobTableValues2 = getAllDatabaseValues(JOB_TABLE);
	for (long i = 0; i < jobTableValues2.size(); i++) {
		long samplesLeft = getRowsLeftFromDatabase(MEASUREMENT_TABLE, measurementIDList.at(i));
		jobTableValues2.at(i).push_back(std::to_string(samplesLeft));
	}
	std::vector<std::string> columnNames = getDatabaseColumnNames(JOB_TABLE);
	columnNames.push_back("Samples left");
	jobTableValues2.insert(jobTableValues2.begin(), columnNames); // On top
}