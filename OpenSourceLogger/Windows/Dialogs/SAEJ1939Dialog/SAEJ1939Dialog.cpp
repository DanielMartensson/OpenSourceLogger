#include "SAEJ1939Dialog.h"
#include "imgui.h"
#include <cstdio>
#include "../../../Hardware/USB/USBHandler.h"
#include "../../../Hardware/USB/Protocols/OpenSourceLogger/OpenSAEJ1939/ISO_11783/ISO_11783-7_Application_Layer/Application_Layer.h"

void inputScalarLimit(const char* label, ImGuiDataType data_type, void* p_data, int min, int max);

void showSAEJ1939OtherECUView(bool* saeJ1939, J1939* j1939) {
	ImGui::SetNextWindowSize(ImVec2(800.0f, 500.0f));
	ImGui::Begin("SAE J1939 for other ECU", saeJ1939, ImGuiWindowFlags_NoResize);
	if (isConnectedToUSB()) {
		static int functionIndex = 0;
		const char* functionNames = "ECU Name\0Software identification\0Component identification\0ECU identification\0Auxilary valve command\0General valve command\0DM1 messages\0DM2 messages\0DM3 messages";
		ImGui::Combo("Select function:", &functionIndex, functionNames);
		ImGui::Separator();

		// Show different view
		switch (functionIndex) {
		case 0: // ECU Name
		{
			static int DA = 0;
			static int newDA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			inputScalarLimit("New ECU Destination address:", ImGuiDataType_U8, &newDA, 0, 253); // 254 is the error address and 255 is broadcast address

			if (ImGui::Button("Send name request")) {
				SAE_J1939_Send_Request_Address_Claimed(j1939, DA);
			}
			ImGui::SameLine();
			if (ImGui::Button("Send name change")) {
				SAE_J1939_Send_Commanded_Address(j1939, DA, newDA,
					j1939->from_other_ecu_name.identity_number,
					j1939->from_other_ecu_name.manufacturer_code,
					j1939->from_other_ecu_name.function_instance,
					j1939->from_other_ecu_name.ECU_instance,
					j1939->from_other_ecu_name.function,
					j1939->from_other_ecu_name.vehicle_system,
					j1939->from_other_ecu_name.arbitrary_address_capable,
					j1939->from_other_ecu_name.industry_group,
					j1939->from_other_ecu_name.vehicle_system_instance);
			}

			// Name from the ECU
			ImGui::Separator();
			inputScalarLimit("Identity number:", ImGuiDataType_U32, &j1939->from_other_ecu_name.identity_number, 0, 2097151);
			inputScalarLimit("Manufacture code:", ImGuiDataType_U16, &j1939->from_other_ecu_name.manufacturer_code, 0, 2047);
			inputScalarLimit("Function instance:", ImGuiDataType_U8, &j1939->from_other_ecu_name.function_instance, 0, 31);
			inputScalarLimit("ECU instance:", ImGuiDataType_U8, &j1939->from_other_ecu_name.ECU_instance, 0, 7);
			inputScalarLimit("Function:", ImGuiDataType_U8, &j1939->from_other_ecu_name.function, 0, 255);
			inputScalarLimit("Vehicle system:", ImGuiDataType_U8, &j1939->from_other_ecu_name.vehicle_system, 0, 127);
			inputScalarLimit("Vehicle system instance:", ImGuiDataType_U8, &j1939->from_other_ecu_name.vehicle_system_instance, 0, 15);
			inputScalarLimit("Industry group:", ImGuiDataType_U8, &j1939->from_other_ecu_name.industry_group, 0, 7);
			inputScalarLimit("Arbitary address capable:", ImGuiDataType_U8, &j1939->from_other_ecu_name.arbitrary_address_capable, 0, 1);
			ImGui::InputInt("From ECU address:", (int*)&j1939->from_other_ecu_name.from_ecu_address, 1, 100, ImGuiInputTextFlags_ReadOnly);

			// Claimed addresses and not claimed addresses
			ImGui::Separator();
			char text[50];
			char address_of_claimed_addresses[500] = { 0 };
			int index = 0;
			for (int i = 0; i < j1939->number_of_other_ECU; i++) {
				sprintf_s(text, "%i", j1939->other_ECU_address[i]);
				for (int j = 0; j < 50; j++) {
					address_of_claimed_addresses[index++] = text[j];
					if (text[j] == 0) {
						break;
					}
				}
			}
			static int addressIndex = 0;
			ImGui::Combo("Number of other claimed ECU addresses", &addressIndex, address_of_claimed_addresses);
			ImGui::InputInt("Number of cannot claim address:", (int*)&j1939->number_of_cannot_claim_address, 1, 100, ImGuiInputTextFlags_ReadOnly);
			break;
		}
		case 1: // Software identification
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			if (ImGui::Button("Send software ID request")) {
				SAE_J1939_Send_Request_Software_Identification(j1939, DA);
			}
			ImGui::InputText("Software ID:", (char*)j1939->from_other_ecu_identifications.software_identification.identifications, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputInt("From ECU address:", (int*)&j1939->from_other_ecu_identifications.software_identification.from_ecu_address, 1, 100, ImGuiInputTextFlags_ReadOnly);
			break;
		}
		case 2: // Component identification
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			if (ImGui::Button("Send component ID request")) {
				SAE_J1939_Send_Request_Component_Identification(j1939, DA);
			}
			ImGui::InputText("Component model name:", (char*)j1939->from_other_ecu_identifications.component_identification.component_model_name, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputText("Component product date:", (char*)j1939->from_other_ecu_identifications.component_identification.component_product_date, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputText("Component serial number:", (char*)j1939->from_other_ecu_identifications.component_identification.component_serial_number, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputText("Component unit name:", (char*)j1939->from_other_ecu_identifications.component_identification.component_unit_name, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputInt("From ECU address:", (int*)&j1939->from_other_ecu_identifications.component_identification.from_ecu_address, 1, 100, ImGuiInputTextFlags_ReadOnly);
			break;
		}
		case 3: // ECU identification
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			if (ImGui::Button("Send ECU ID request")) {
				SAE_J1939_Send_Request_ECU_Identification(j1939, DA);
			}
			ImGui::InputText("ECU location:", (char*)j1939->from_other_ecu_identifications.ecu_identification.ecu_location, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputText("ECU part number:", (char*)j1939->from_other_ecu_identifications.ecu_identification.ecu_part_number, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputText("ECU serial number:", (char*)j1939->from_other_ecu_identifications.ecu_identification.ecu_serial_number, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputText("ECU type:", (char*)j1939->from_other_ecu_identifications.ecu_identification.ecu_type, MAX_IDENTIFICATION, ImGuiInputTextFlags_ReadOnly);
			ImGui::InputInt("From ECU address:", (int*)&j1939->from_other_ecu_identifications.ecu_identification.from_ecu_address, 1, 100, ImGuiInputTextFlags_ReadOnly);
			break;
		}
		case 4: // Auxilary valve command
		{
			char text[25];
			static bool auxiliaryValveActive[16] = { false };
			static int auxiliaryValveFlow[16] = { 0 };
			static int auxiliaryValveState[16] = { 0 };
			for (int i = 0; i < 16;) {
				sprintf_s(text, "Auxiliary valve: %i", i);
				ImGui::Checkbox(text, &auxiliaryValveActive[i++]);
				ImGui::SameLine();
				sprintf_s(text, "Auxiliary valve: %i", i);
				ImGui::Checkbox(text, &auxiliaryValveActive[i++]);
			}

			// Time difference
			static std::chrono::steady_clock::time_point t1;
			static std::chrono::steady_clock::time_point t2;
			t2 = std::chrono::steady_clock::now();
			static long timeDifferenceSum = 0;
			timeDifferenceSum += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			t1 = t2;

			// Display sliders
			for (int i = 0; i < 16; i++) {
				if (auxiliaryValveActive[i]) {
					sprintf_s(text, "Auxilliary valve: %i", i);
					ImGui::SliderInt(text, &auxiliaryValveFlow[i], -250, 250);
					int valveState = VALVE_STATE_NEUTRAL;
					if (auxiliaryValveFlow[i] > 0) {
						valveState = VALVE_STATE_EXTEND;
					}
					if (auxiliaryValveFlow[i] < 0) {
						valveState = VALVE_STATE_RETRACT;
					}
					auxiliaryValveState[i] = valveState;
				}
			}

			// For every 50 milliseconds, send the CAN-messagess
			if (timeDifferenceSum > 50) {
				for (int i = 0; i < 16; i++) {
					if (auxiliaryValveActive[i]) {
						ISO_11783_Send_Auxiliary_Valve_Command(j1939, i, auxiliaryValveFlow[i], FAIL_SAFE_MODE_BLOCKED, auxiliaryValveState[i]);
					}
				}
				timeDifferenceSum = 0;
			}
			break;
		}
		case 5: // General valve command
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address

			// Time difference
			static std::chrono::steady_clock::time_point t1;
			static std::chrono::steady_clock::time_point t2;
			t2 = std::chrono::steady_clock::now();
			static long timeDifferenceSum = 0;
			timeDifferenceSum += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
			t1 = t2;

			// Show slider
			static int standardFlow = 0;
			ImGui::SliderInt("General valve command:", &standardFlow, -250, 250);
			int valveState = VALVE_STATE_NEUTRAL;
			if (standardFlow > 0) {
				valveState = VALVE_STATE_EXTEND;
			}
			if (standardFlow < 0) {
				valveState = VALVE_STATE_RETRACT;
			}

			// For every 50 milliseconds, send the CAN-messagess
			if (timeDifferenceSum > 50) {
				ISO_11783_Send_General_Purpose_Valve_Command(j1939, DA, standardFlow, FAIL_SAFE_MODE_BLOCKED, valveState, 0);
				timeDifferenceSum = 0;
			}
			break;
		}
		case 6: // DM1 messages
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			if (ImGui::Button("Send DM1 request")) {
				SAE_J1939_Send_Request_DM1(j1939, DA);
			}
			if (j1939->from_other_ecu_dm.errors_dm1_active > 0) {
				// At least one error...
				static int errorIndex = 1;
				if (ImGui::InputInt("Select DM1 error:", &errorIndex)) {
					if (errorIndex < 1) {
						errorIndex = 1;
					}
					if (errorIndex > j1939->from_other_ecu_dm.errors_dm1_active) {
						errorIndex = j1939->from_other_ecu_dm.errors_dm1_active;
					}
				}
				ImGui::InputInt("FMI:", (int*)&j1939->from_other_ecu_dm.dm1.FMI[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("Occurrence count:", (int*)&j1939->from_other_ecu_dm.dm1.occurrence_count[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SPN:", (int*)&j1939->from_other_ecu_dm.dm1.SPN[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SPN conversion method:", (int*)&j1939->from_other_ecu_dm.dm1.SPN_conversion_method[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp amber warning:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_flash_lamp_amber_warning, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp malfunction indicator:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_flash_lamp_malfunction_indicator, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp protect lamp:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_flash_lamp_protect_lamp, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp red stop:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_flash_lamp_red_stop, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status amber warning:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_lamp_status_amber_warning, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status malfunction indicator:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_lamp_status_malfunction_indicator, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status protect lamp:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_lamp_status_protect_lamp, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status red stop:", (int*)&j1939->from_other_ecu_dm.dm1.SAE_lamp_status_red_stop, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("From ECU address:", (int*)&j1939->from_other_ecu_dm.dm1.from_ecu_address[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
			}
			else {
				ImGui::Text("No DM1 errors available");
			}
			break;
		}

		case 7: // DM2 messages 
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			if (ImGui::Button("Send DM2 request")) {
				SAE_J1939_Send_Request_DM2(j1939, DA);
			}
			if (j1939->from_other_ecu_dm.errors_dm2_active > 0) {
				// At least one error...
				static int errorIndex = 1;
				if (ImGui::InputInt("Select DM2 error:", &errorIndex)) {
					if (errorIndex < 1) {
						errorIndex = 1;
					}
					if (errorIndex > j1939->from_other_ecu_dm.errors_dm2_active) {
						errorIndex = j1939->from_other_ecu_dm.errors_dm2_active;
					}
				}
				ImGui::InputInt("FMI:", (int*)&j1939->from_other_ecu_dm.dm2.FMI[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("Occurrence count:", (int*)&j1939->from_other_ecu_dm.dm2.occurrence_count[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SPN:", (int*)&j1939->from_other_ecu_dm.dm2.SPN[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SPN conversion method:", (int*)&j1939->from_other_ecu_dm.dm2.SPN_conversion_method[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp amber warning:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_flash_lamp_amber_warning, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp malfunction indicator:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_flash_lamp_malfunction_indicator, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp protect lamp:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_flash_lamp_protect_lamp, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE flash lamp red stop:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_flash_lamp_red_stop, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status amber warning:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_lamp_status_amber_warning, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status malfunction indicator:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_lamp_status_malfunction_indicator, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status protect lamp:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_lamp_status_protect_lamp, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("SAE lamp status red stop:", (int*)&j1939->from_other_ecu_dm.dm2.SAE_lamp_status_red_stop, 1, 100, ImGuiInputTextFlags_ReadOnly);
				ImGui::InputInt("From ECU address:", (int*)&j1939->from_other_ecu_dm.dm2.from_ecu_address[errorIndex], 1, 100, ImGuiInputTextFlags_ReadOnly);
			}
			else {
				ImGui::Text("No DM2 errors available");
			}
			break;
		}
		case 8: // DM3 messages
		{
			static int DA = 0;
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &DA, 0, 253); // 254 is the error address and 255 is broadcast address
			if (ImGui::Button("Send DM3 request")) {
				SAE_J1939_Send_Request_DM3(j1939, DA);
			}
			ImGui::Text("DM3 messages will clear DM2 messages");
		}
		}
	}else {
		ImGui::Text("You need to be connected to the USB");
	}

	ImGui::End();
}

void showSAEJ1939ThisECUView(bool* saeJ1939, J1939* j1939) {
	ImGui::SetNextWindowSize(ImVec2(800.0f, 500.0f));
	ImGui::Begin("SAE J1939 for this ECU", saeJ1939, ImGuiWindowFlags_NoResize);
	if (isConnectedToUSB()) {
		static int functionIndex = 0;
		const char* functionNames = "ECU Name\0Software identification\0Component identification\0ECU identification\0DM1 messages\0DM2 messages";
		ImGui::Combo("Select function:", &functionIndex, functionNames);
		ImGui::Separator();

		// Show different view
		switch (functionIndex) {
		case 0: // ECU Name
		{
			inputScalarLimit("ECU Destination address:", ImGuiDataType_U8, &j1939->information_this_ECU.this_ECU_address, 0, 253); // 254 is the error address and 255 is broadcast address
			inputScalarLimit("Identity number:", ImGuiDataType_U32, &j1939->information_this_ECU.this_name.identity_number, 0, 2097151);
			inputScalarLimit("Manufacture code:", ImGuiDataType_U16, &j1939->information_this_ECU.this_name.manufacturer_code, 0, 2047);
			inputScalarLimit("Function instance:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.function_instance, 0, 31);
			inputScalarLimit("ECU instance:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.ECU_instance, 0, 7);
			inputScalarLimit("Function:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.function, 0, 255);
			inputScalarLimit("Vehicle system:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.vehicle_system, 0, 127);
			inputScalarLimit("Vehicle system instance:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.vehicle_system_instance, 0, 15);
			inputScalarLimit("Industry group:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.industry_group, 0, 7);
			inputScalarLimit("Arbitary address capable:", ImGuiDataType_U8, &j1939->information_this_ECU.this_name.arbitrary_address_capable, 0, 1);
			break;
		}
		case 1: // Software identification
		{
			ImGui::InputText("Software ID:", (char*)j1939->information_this_ECU.this_identifications.software_identification.identifications, MAX_IDENTIFICATION);
			break;
		}
		case 2: // Component identification
		{
			ImGui::InputText("Component model name:", (char*)j1939->information_this_ECU.this_identifications.component_identification.component_model_name, MAX_IDENTIFICATION);
			ImGui::InputText("Component product date:", (char*)j1939->information_this_ECU.this_identifications.component_identification.component_product_date, MAX_IDENTIFICATION);
			ImGui::InputText("Component serial number:", (char*)j1939->information_this_ECU.this_identifications.component_identification.component_serial_number, MAX_IDENTIFICATION);
			ImGui::InputText("Component unit name:", (char*)j1939->information_this_ECU.this_identifications.component_identification.component_unit_name, MAX_IDENTIFICATION);
			break;
		}
		case 3: // ECU identification
		{
			ImGui::InputText("ECU location:", (char*)j1939->information_this_ECU.this_identifications.ecu_identification.ecu_location, MAX_IDENTIFICATION);
			ImGui::InputText("ECU part number:", (char*)j1939->information_this_ECU.this_identifications.ecu_identification.ecu_part_number, MAX_IDENTIFICATION);
			ImGui::InputText("ECU serial number:", (char*)j1939->information_this_ECU.this_identifications.ecu_identification.ecu_serial_number, MAX_IDENTIFICATION);
			ImGui::InputText("ECU type:", (char*)j1939->information_this_ECU.this_identifications.ecu_identification.ecu_type, MAX_IDENTIFICATION);
			break;
		}
		case 4: // DM1 messages
		{
			inputScalarLimit("DM1 errors active:", ImGuiDataType_U8, &j1939->this_dm.errors_dm1_active, 0, MAX_DM_FIELD);
			if (j1939->this_dm.errors_dm1_active > 0) {
				// At least one error...
				static int errorIndex = 1;
				if (ImGui::InputInt("Select DM1 error:", &errorIndex)) {
					if (errorIndex < 1) {
						errorIndex = 1;
					}
					if (errorIndex > j1939->this_dm.errors_dm1_active) {
						errorIndex = j1939->this_dm.errors_dm1_active;
					}
				}
				inputScalarLimit("FMI:", ImGuiDataType_U8, &j1939->this_dm.dm1.FMI[errorIndex], 0, 255);
				inputScalarLimit("Occurrence count:", ImGuiDataType_U8, &j1939->this_dm.dm1.occurrence_count[errorIndex], 0, 126);
				inputScalarLimit("SPN:", ImGuiDataType_U32, &j1939->this_dm.dm1.SPN[errorIndex], 0, UINT_MAX);
				inputScalarLimit("SPN conversion method:", ImGuiDataType_U8, &j1939->this_dm.dm1.SPN_conversion_method[errorIndex], 0, 1);
				inputScalarLimit("SAE flash lamp amber warning:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_flash_lamp_amber_warning, 0, 1);
				inputScalarLimit("SAE flash lamp malfunction indicator:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_flash_lamp_malfunction_indicator, 0, 1);
				inputScalarLimit("SAE flash lamp protect lamp:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_flash_lamp_protect_lamp, 0, 1);
				inputScalarLimit("SAE flash lamp red stop:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_flash_lamp_red_stop, 0, 1);
				inputScalarLimit("SAE lamp status amber warning:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_lamp_status_amber_warning, 0, 1);
				inputScalarLimit("SAE lamp status malfunction indicator:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_lamp_status_malfunction_indicator, 0, 1);
				inputScalarLimit("SAE lamp status protect lamp:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_lamp_status_protect_lamp, 0, 1);
				inputScalarLimit("SAE lamp status red stop:", ImGuiDataType_U8, &j1939->this_dm.dm1.SAE_lamp_status_red_stop, 0, 1);
			}
			else {
				ImGui::Text("No DM1 errors active");
			}
			break;
		}

		case 5: // DM2 messages 
		{
			inputScalarLimit("DM2 errors active:", ImGuiDataType_U8, &j1939->this_dm.errors_dm2_active, 0, MAX_DM_FIELD);
			if (j1939->this_dm.errors_dm2_active > 0) {
				// At least one error...
				static int errorIndex = 1;
				if (ImGui::InputInt("Select DM2 error:", &errorIndex)) {
					if (errorIndex < 1) {
						errorIndex = 1;
					}
					if (errorIndex > j1939->this_dm.errors_dm2_active) {
						errorIndex = j1939->this_dm.errors_dm2_active;
					}
				}
				inputScalarLimit("FMI:", ImGuiDataType_U8, &j1939->this_dm.dm2.FMI[errorIndex], 0, 255);
				inputScalarLimit("Occurrence count:", ImGuiDataType_U8, &j1939->this_dm.dm2.occurrence_count[errorIndex], 0, 126);
				inputScalarLimit("SPN:", ImGuiDataType_U32, &j1939->this_dm.dm2.SPN[errorIndex], 0, UINT_MAX);
				inputScalarLimit("SPN conversion method:", ImGuiDataType_U8, &j1939->this_dm.dm2.SPN_conversion_method[errorIndex], 0, 1);
				inputScalarLimit("SAE flash lamp amber warning:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_flash_lamp_amber_warning, 0, 1);
				inputScalarLimit("SAE flash lamp malfunction indicator:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_flash_lamp_malfunction_indicator, 0, 1);
				inputScalarLimit("SAE flash lamp protect lamp:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_flash_lamp_protect_lamp, 0, 1);
				inputScalarLimit("SAE flash lamp red stop:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_flash_lamp_red_stop, 0, 1);
				inputScalarLimit("SAE lamp status amber warning:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_lamp_status_amber_warning, 0, 1);
				inputScalarLimit("SAE lamp status malfunction indicator:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_lamp_status_malfunction_indicator, 0, 1);
				inputScalarLimit("SAE lamp status protect lamp:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_lamp_status_protect_lamp, 0, 1);
				inputScalarLimit("SAE lamp status red stop:", ImGuiDataType_U8, &j1939->this_dm.dm2.SAE_lamp_status_red_stop, 0, 1);
			}
			else {
				ImGui::Text("No DM2 errors active");
			}
			break;
		}
		}
	}
	else {
		ImGui::Text("You need to be connected to the USB");
	}

	ImGui::End();

	// Save J1939 structure
	if (*saeJ1939 == false) {
		uint32_t ECU_information_length = sizeof(Information_this_ECU);
		uint8_t ECU_information_data[sizeof(Information_this_ECU)];
		memcpy(ECU_information_data, (uint8_t*)&j1939->information_this_ECU, ECU_information_length);
		Save_Struct(ECU_information_data, ECU_information_length, (char*)INFORMATION_THIS_ECU);
	}

}

void inputScalarLimit(const char* label, ImGuiDataType data_type, void* p_data, int min, int max) {
	if (ImGui::InputScalar(label, data_type, p_data)) {
		switch (data_type) {
		case ImGuiDataType_U8:
			if (*((uint8_t*)p_data) < min) {
				*((uint8_t*)p_data) = min;
			}
			if (*((uint8_t*)p_data) > max) {
				*((uint8_t*)p_data) = max;
			}
			break;
		case ImGuiDataType_U16:
			if (*((uint16_t*)p_data) < min) {
				*((uint16_t*)p_data) = min;
			}
			if (*((uint16_t*)p_data) > max) {
				*((uint16_t*)p_data) = max;
			}
			break;
		case ImGuiDataType_U32:
			if (*((uint32_t*)p_data) < min) {
				*((uint32_t*)p_data) = min;
			}
			if (*((uint32_t*)p_data) > max) {
				*((uint32_t*)p_data) = max;
			}
			break;
		}

	}
}