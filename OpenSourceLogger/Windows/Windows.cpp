#include "Windows.h"
#include <imgui.h>
#include "../Hardware/USB/USBHandler.h"
#include "Dialogs/FileDialog/FileDialog.h"
#include "Dialogs/ConnectDialog/ConnectDialog.h"
#include "Dialogs/CalibrationDialog/CalibrationDialog.h"
#include "Dialogs/CANDialogs/CANBusDialog/CANBusDialog.h"
#include "Dialogs/CANDialogs/SAEJ1939Dialog/SAEJ1939Dialog.h"
#include "Dialogs/PinMapOfSTM32PLCDialog/PinMapOfSTM32PLCDialog.h"
#include "Dialogs/CollectMeasurementsDialog/CollectMeasurementsDialog.h"
#include "Dialogs/ConfigurationSTM32PLCDialog/ConfigurationSTM32PLCDialog.h"
#include "Dialogs/UploadMeasurementToDatabaseDialog/UploadMeasurementToDatabaseDialog.h"
#include "Dialogs/ViewMeasurementsFromDatabaseDialog/ViewMeasurementsFromDatabaseDialog.h"

// Window states
bool mainWindowClosed = false;
bool createOrSelectMeasurement = false;
bool uploadMeasurementToDatabase = false;
bool connectToUsb = false;
bool connectToDatabase = false;
bool configureSTM32PLC = false;
bool calibrateMeasurements = false;
bool collectMeasurements = false;
bool measurementsFromDatabase = false;
bool saeJ1939OtherECU = false;
bool saeJ1939ThisECU = false;
bool canBusTerminal = false;
bool pinMapOfSTM32PLC = false;

// Window properties
char file_folder_path[1000];

// SAE J1939 struct
J1939 j1939 = { 0 }; 
bool hasECUBeenStartedUp = false;


void showMainWindow(bool* show_main_window) {
	// Show the main window
	*show_main_window = true;
	ImGui::SetNextWindowSize(ImVec2(500.0f, 350.0f));
	ImGui::Begin("Main window", show_main_window, ImGuiWindowFlags_NoResize);

	// Show collapsing headers
	if (ImGui::CollapsingHeader("File")) {
		if (ImGui::Button("Create or select measurement")) {
			createOrSelectMeasurement = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Upload measurement to database")) {
			uploadMeasurementToDatabase = true;
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
	}
	if (ImGui::CollapsingHeader("CAN Bus")) {
		if (ImGui::Button("SAE J1939 Other ECU")) {
			saeJ1939OtherECU = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("SAE J1939 This ECU")) {
			saeJ1939ThisECU = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("CAN bus terminal")) {
			canBusTerminal = true;
		}
	}

	if (ImGui::CollapsingHeader("Help")) {
		if (ImGui::Button("Pin map of STM32PLC")) {
			pinMapOfSTM32PLC = true;
		}
	}
	ImGui::End();

	// Load J1939
	if (!hasECUBeenStartedUp && isConnectedToUSB()) {
		Open_SAE_J1939_Startup_ECU(&j1939);
		hasECUBeenStartedUp = true;
	}
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
		// Open createTables calibration
		showCalibrationDialog(&calibrateMeasurements);
	}
	if (collectMeasurements) {
		// Open collect measurements dialog
		showCollectMeasurementsDialog(&collectMeasurements, file_folder_path);
	}
	if (configureSTM32PLC) {
		// Open STM32 configuration
		showConfigurationSTM32PLCDialog(&configureSTM32PLC);
	}
	if (uploadMeasurementToDatabase) {
		// Open upload measurement to the database
		showUploadMeasurementToDatabaseDialog(&uploadMeasurementToDatabase);
	}
	if (measurementsFromDatabase) {
		// Open view measurements from database
		showViewMeasurementFromDatabaseDialog(&measurementsFromDatabase);
	}
	if (saeJ1939OtherECU) {
		// Open view SAE J1939 window for other ECU
		showSAEJ1939OtherECUView(&saeJ1939OtherECU, &j1939);
	}
	if (saeJ1939ThisECU) {
		// Open view SAE J1939 window for this ECU
		showSAEJ1939ThisECUView(&saeJ1939ThisECU, &j1939);
	}
	if (pinMapOfSTM32PLC) {
		// Show pin map of STM32PLC
		showPinMapOfSTM32PLCDialog(&pinMapOfSTM32PLC);
	}
	if (canBusTerminal) {
		// Show CAN-bus terminal
		showCANBusTerminal(&canBusTerminal);
	}
}