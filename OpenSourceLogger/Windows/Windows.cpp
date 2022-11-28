#include "Windows.h"
#include <imgui.h>
#include "Dialogs/FileDialog/FileDialog.h"
#include "Dialogs/ConnectDialog/ConnectDialog.h"
#include "Dialogs/CalibrationDialog/CalibrationDialog.h"
#include "Dialogs/CollectMeasurementsDialog/CollectMeasurementsDialog.h"

// Window states
static bool mainWindowClosed = false;
static bool createOrSelectMeasurement = false;
static bool uploadMeasurementToDatabase = false;
static bool deleteMeasurementFromDatabase = false;
static bool connectToUsb = false;
static bool connectToDatabase = false;
static bool configureIoPins = false;
static bool configureSTM32PLC = false;
static bool calibrateMeasurements = false;
static bool collectMeasurements = false;
static bool measurementsFromDatabase = false;
static bool measurementsFromCsvFile = false;
static bool manualOfOpenSourceLogger = false;
static bool pinMapOfSTM32PLC = false;

// Window properties
static char file_folder_path[100];

// SAE J1939 struct
J1939 j1939;


void showMainWindow(bool* show_main_window) {
	// Show the main window
	*show_main_window = true;
	ImGui::Begin("Main window", show_main_window);

	// Show collapsing headers
	if (ImGui::CollapsingHeader("File")) {
		if (ImGui::Button("Create or select measurement")) {
			createOrSelectMeasurement = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Upload measurement to database")) {
			uploadMeasurementToDatabase = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete measurement from database")) {
			deleteMeasurementFromDatabase = true;
		}
	}

	if (ImGui::CollapsingHeader("Connection")) {
		if (ImGui::Button("Connect to USB")) {
			connectToUsb = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Connect to database")) {
			connectToDatabase = true;
		}
	}

	if (ImGui::CollapsingHeader("Configuration")) {
		if (ImGui::Button("Configure I/O pins")) {
			configureIoPins = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Configure STM32PLC")) {
			configureSTM32PLC = true;
		}
	}

	if (ImGui::CollapsingHeader("Measurement")) {
		if (ImGui::Button("Calibrate measurements")) {
			calibrateMeasurements = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Collect measurements")) {
			collectMeasurements = true;
		}
	}

	if (ImGui::CollapsingHeader("View")) {
		if (ImGui::Button("Measurements from database")) {
			measurementsFromDatabase = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Measurements from CSV file")) {
			measurementsFromCsvFile = true;
		}
	}

	if (ImGui::CollapsingHeader("Help")) {
		if (ImGui::Button("Manual of OpenSourceLogger")) {
			manualOfOpenSourceLogger = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Pin map of STM32PLC")) {
			pinMapOfSTM32PLC = true;
		}
	}

	ImGui::End();
}

void showDialogWindows() {
	if (createOrSelectMeasurement) {
		// Open new file dialog for file handling
		showFileDialog(&createOrSelectMeasurement, file_folder_path, sizeof(file_folder_path), FileDialogType::OpenFile);
	}
	if (connectToUsb) {
		// Open connection dialog for USB
		showUSBConnectionDialog(&connectToUsb, &j1939);
	}
	if (connectToDatabase) {
		// Open connection dialog for database
		showDatabaseConnectionDialog(&connectToDatabase);
	}
	if (calibrateMeasurements) {
		// Open createTable calibration
		showCalibrationDialog(&calibrateMeasurements);
	}
	if (collectMeasurements) {
		// Open collect measurements dialog
		showCollectMeasurementsDialog(&collectMeasurements, file_folder_path);
	}
}