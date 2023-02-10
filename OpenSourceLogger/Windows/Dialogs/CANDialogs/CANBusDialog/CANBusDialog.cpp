#include "CANBusDialog.h"
#include "imgui.h"
#include "../../../../Hardware/USB/USBHandler.h"
#include "../../../../Hardware/USB/Protocols/OpenSourceLogger/OpenSourceLoggerProtocol.h"

std::vector<std::string> direction;
std::vector<uint8_t> canType;
std::vector<uint32_t> canID;
std::vector<uint8_t> canDLC;
std::vector<std::vector<uint8_t>> canData;

void getMessage(uint8_t CAN_BUS_MESSAGE[], bool isRxData);
void clearTable();

void showCANBusTerminal(bool* canBusTerminal) {
	ImGui::SetNextWindowSize(ImVec2(1000.f, 440.0f));
	ImGui::Begin("CAN Terminal", canBusTerminal, ImGuiWindowFlags_NoResize);
	if (isConnectedToUSB()) {
		// Get data
		bool isNewMessage = false;
		uint8_t* CAN_BUS_MESSAGE = getRxCanBusMessage(&isNewMessage);
		if (isNewMessage)
			getMessage(CAN_BUS_MESSAGE, true);
		isNewMessage = false;
		CAN_BUS_MESSAGE = getTxCanBusMessage(&isNewMessage);
		if (isNewMessage)
			getMessage(CAN_BUS_MESSAGE, false);

		// Clear table
		if(ImGui::Button("Clear table")) {
			clearTable();
		}

		// Create table
		ImGui::BeginTable("canTerminal", 12, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner);
		ImGui::TableSetupColumn("Direction");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("ID");
		ImGui::TableSetupColumn("DLC");
		ImGui::TableSetupColumn("D0");
		ImGui::TableSetupColumn("D1");
		ImGui::TableSetupColumn("D2");
		ImGui::TableSetupColumn("D3");
		ImGui::TableSetupColumn("D4");
		ImGui::TableSetupColumn("D5");
		ImGui::TableSetupColumn("D6");
		ImGui::TableSetupColumn("D7");
		ImGui::TableHeadersRow();

		for (int i = 0; i < canType.size(); i++) {
			// New row
			char text[20];
			ImGui::TableNextRow();

			// Direction
			ImGui::TableSetColumnIndex(0);
			sprintf_s(text, "%s", direction.at(i).c_str());
			ImGui::Text(text);

			// Type column
			ImGui::TableSetColumnIndex(1);
			sprintf_s(text, "%s", canType.at(i) == CAN_ID_STD ? "STD" : "EXT");
			ImGui::Text(text);

			// ID column
			ImGui::TableSetColumnIndex(2);
			sprintf_s(text, "0x%X", canID.at(i));
			ImGui::Text(text);

			// DLC column
			ImGui::TableSetColumnIndex(3);
			sprintf_s(text, "0x%X", canDLC.at(i));
			ImGui::Text(text);

			// Data columns
			for (int j = 0; j < canData.at(i).size(); j++) {
				ImGui::TableSetColumnIndex(4 + j);
				sprintf_s(text, "0x%X", canData.at(i).at(j));
				ImGui::Text(text);
			}
		}
		ImGui::EndTable();

		// Size of the rows
		if (canType.size() > 20) {
			direction.erase(direction.begin());
			canType.erase(canType.begin());
			canID.erase(canID.begin());
			canDLC.erase(canDLC.begin());
			canData.erase(canData.begin());
		}

	}
	else {
		clearTable();
		ImGui::Text("You need to be connected to the USB");
	}
	ImGui::End();
}

void getMessage(uint8_t CAN_BUS_MESSAGE[], bool isRxData) {
	direction.push_back(isRxData == true ? std::string("RX") : std::string("TX"));
	canType.push_back(CAN_BUS_MESSAGE[0]);
	canID.push_back((CAN_BUS_MESSAGE[1] << 24) | (CAN_BUS_MESSAGE[2] << 16) | (CAN_BUS_MESSAGE[3] << 8) | CAN_BUS_MESSAGE[4]);
	uint8_t DLC = CAN_BUS_MESSAGE[5];
	canDLC.push_back(DLC);
	std::vector<uint8_t> data;
	for (int i = 0; i < DLC; i++) {
		data.push_back(CAN_BUS_MESSAGE[6 + i]);
	}
	canData.push_back(data);
}

void clearTable() {
	direction.clear();
	canType.clear();
	canID.clear();
	canDLC.clear();
	canData.clear();
}