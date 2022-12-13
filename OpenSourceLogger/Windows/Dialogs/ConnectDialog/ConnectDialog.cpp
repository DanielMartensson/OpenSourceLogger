#include "ConnectDialog.h"
#include <imgui.h>
#include "imgui_internal.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "../../../Hardware/USB/USBHandler.h"
#include "../../../Hardware/Database/DatabaseHandler.h"
#include "../../../Constants.h"

#define CONNECTION_FIELDS_JSON "Windows/Dialogs/ConnectDialog/connectionFields.json"

inline void create_C_array_for_combo(char array_C[], std::vector<std::string> array) {
	int index = 0;
	for (int i = 0; i < array.size(); i++) {
		for (int j = 0; j < array.at(i).length(); j++) {
			array_C[index++] = array.at(i).at(j);
		}
		array_C[index++] = '\0'; // Important for separator at the ComboBox
	}
}

void showUSBConnectionDialog(bool* connectToUsb, J1939* j1939) {
	// Display
	ImGui::SetNextWindowSize(ImVec2(300.0f, 150.0f));
	ImGui::Begin("Select USB port", connectToUsb, ImGuiWindowFlags_NoResize);

	// Center for popup
	ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);

	// Status
	char status[25];
	sprintf_s(status, "Status: %s", isConnectedToUSB() == true ? "Connected" : "Disconnected");
	ImGui::Text(status);

	// Baudrate combobox fields
	std::vector<std::string> baudrates;
	char baudrates_C_style[100] = { 0 };
	baudrates.push_back("110");
	baudrates.push_back("150");
	baudrates.push_back("300");
	baudrates.push_back("600");
	baudrates.push_back("1200");
	baudrates.push_back("1800");
	baudrates.push_back("2400");
	baudrates.push_back("4800");
	baudrates.push_back("9600");
	baudrates.push_back("19200");
	baudrates.push_back("38400");
	baudrates.push_back("57600");
	baudrates.push_back("115200");
	create_C_array_for_combo(baudrates_C_style, baudrates);

	// Port combobox fields
	static int portIndex = 0;
	static int baudrateIndex = baudrates.size() - 1; // Last index
	static char portNames_C_style[1000] = { 0 };
	static std::vector<std::string> portNames;

	if (isConnectedToUSB()) {
		// Make the ComboBox "disabled" for port
		const char* itemsPorts[] = { portNames.at(portIndex).c_str() };
		int item_current_ports = 1;
		ImGui::ListBox("USB Device", &item_current_ports, itemsPorts, IM_ARRAYSIZE(itemsPorts));

		// Make the ComboBox "disabled" for baudrate
		const char* itemsBaudrates[] = { baudrates.at(baudrateIndex).c_str() };
		int item_current_baudrates = 1;
		ImGui::ListBox("Baudrates", &item_current_baudrates, itemsBaudrates, IM_ARRAYSIZE(itemsBaudrates));
	}
	else {
		ImGui::Combo("USB device", &portIndex, portNames_C_style);
		ImGui::Combo("Baudrate", &baudrateIndex, baudrates_C_style);
	}

	// Color for connect button
	if (!isConnectedToUSB()) {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_RED);
	}else {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_GREEN);
	}

	// Connect button
	if (ImGui::Button("Connect")) {
		if (portNames.size() > 0) {
			if (openUSBConnection(portNames.at(portIndex), baudrateIndex, j1939)) {
				ImGui::OpenPopup("Connected");
			}
		}
	}
	ImGui::PopStyleColor();

	// Pop up for connect button
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopup("Connected", ImGuiWindowFlags_Modal)) {
		ImGui::Text("You are connected to the USB");
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();

	// Color for disconnect button
	if (isConnectedToUSB()) {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_RED);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_GREEN);
	}

	// Disconnection
	if (ImGui::Button("Disconnect") && isConnectedToUSB()) {
		if (closeUSBConnection()) {
			ImGui::OpenPopup("Disconnected");
		}
	}
	ImGui::PopStyleColor();
	
	// Pop up for disconnect button
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopup("Disconnected", ImGuiWindowFlags_Modal)) {
		ImGui::Text("You are disconnected from the USB");
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();

	// Scan the ports
	if (ImGui::Button("Scan for ports") && !isConnectedToUSB()) {
		portNames.clear();
		getUSBPortNames(portNames);
		create_C_array_for_combo(portNames_C_style, portNames);
	}
	ImGui::End();
}

void showDatabaseConnectionDialog(bool* connectToDatabase) {
	// New window
	ImGui::SetNextWindowSize(ImVec2(370.0f, 200.0f));
	ImGui::Begin("Connect to database", connectToDatabase, ImGuiWindowFlags_NoResize);

	// Status
	char status[50];
	sprintf_s(status, "Status: %s", isConnectedToDatabase() == true ? "Connected" : "Disconnected");
	ImGui::Text(status);

	// Input fields - Default values
	static std::string host = "127.0.0.1";
	static std::string schema = "opensourcelogger";
	static int port = 33060;
	static std::string username = "myUser";
	static std::string password = "myPassword";
	static bool fieldsHasBeenLoaded = false;

	// Read only once
	if (!fieldsHasBeenLoaded && !isConnectedToDatabase()) {
		std::ifstream connectionFields;
		connectionFields.open(CONNECTION_FIELDS_JSON);
		if (connectionFields.is_open()) {
			// Read json
			std::string jsonLine;
			std::getline(connectionFields, jsonLine);
			nlohmann::json j = nlohmann::json::parse(jsonLine);

			// Sort it out
			host = j["host"].get<std::string>();
			schema = j["schema"].get<std::string>();
			port = j["port"].get<int>();
			username = j["username"].get<std::string>();
			password = j["password"].get<std::string>();
		}
		connectionFields.close();

		// Lock
		fieldsHasBeenLoaded = true;
	}

	// Display input fields
	ImGui::InputText("Host", host.data(), 50, ImGuiInputTextFlags_CharsNoBlank);
	ImGui::InputText("Schema", schema.data(), 50, ImGuiInputTextFlags_CharsNoBlank);
	ImGui::InputInt("Port", &port);
	ImGui::InputText("Username", username.data(), 50, ImGuiInputTextFlags_CharsNoBlank);
	ImGui::InputText("Password", password.data(), 50, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);

	// Center for popup
	ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5f, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5f);

	// Color for connect button
	if (!isConnectedToDatabase()) {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_RED);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_GREEN);
	}

	// Connect button
	if (ImGui::Button("Connect")) {
		if (openDatabaseConnection(host.data(), port, username.data(), password.data(), schema.data())) {
			// Save Json
			nlohmann::json j;
			j["host"] = host;
			j["schema"] = schema;
			j["port"] = port;
			j["username"] = username;
			j["password"] = password;
			std::string json = j.dump();

			// Write
			std::ofstream connectionFields;
			connectionFields.open(CONNECTION_FIELDS_JSON);
			if (connectionFields.is_open()) {
				connectionFields << json;
			}
			connectionFields.close();

			// Remove lock
			fieldsHasBeenLoaded = false;

			// Activate popup
			ImGui::OpenPopup("Connected");
		}
		else {
			ImGui::OpenPopup("Fail");
		}
	}
	ImGui::PopStyleColor();

	// Popup for connect button
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopup("Connected", ImGuiWindowFlags_Modal)) {
		ImGui::Text("You are connected to the database");
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopup("Fail", ImGuiWindowFlags_Modal)) {
		ImGui::Text("You failed to connect from the database");
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// Collor for disconnect button
	if (isConnectedToDatabase()) {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_RED);
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, COLOR_GREEN);
	}
	ImGui::SameLine();
	if (ImGui::Button("Disconnect") && isConnectedToDatabase()) {
		if (closeDatabaseConnection()) {
			ImGui::OpenPopup("Disconnected");
		}
	}
	ImGui::PopStyleColor();
	
	// Pop up for disconnect button
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopup("Disconnected", ImGuiWindowFlags_Modal)) {
		ImGui::Text("You are disconnected to the database");
		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();

	// Delete schema
	if (isConnectedToDatabase()) {
		if (ImGui::Button("Delete schema")) {
			ImGui::OpenPopup("DeleteSchema");
		}
	}
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopup("DeleteSchema", ImGuiWindowFlags_Modal)) {
		char text[100];
		sprintf_s(text, "Do you want to delete the schema name: %s", schema.data());
		ImGui::Text(text);
		static bool deleteSchemaConfirm;
		ImGui::Checkbox("Yes, I want to delete the schema", &deleteSchemaConfirm);
		if (ImGui::Button("Cancle")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (deleteSchemaConfirm) {
			if (ImGui::Button("Delete now")) {
				dropDatabaseSchema(schema.data());
				closeDatabaseConnection();
				ImGui::CloseCurrentPopup();
				deleteSchemaConfirm = false;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::End();
}