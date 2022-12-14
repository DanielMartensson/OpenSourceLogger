#include "ConfigurationSTM32PLCDialog.h"
#include "imgui.h"
#include <cstdio>
#include "../../../Hardware/USB/USBHandler.h"

void showConfigurationSTM32PLCDialog(bool* configureSTM32PLC) {
	ImGui::SetNextWindowSize(ImVec2(750.0f, 500.0f));
	ImGui::Begin("Configure STM32PLC", configureSTM32PLC, ImGuiWindowFlags_NoResize);

	if (isConnectedToUSB()) {
		ImGui::Text("PWM Prescalers:");
		ImGui::BeginChild("pwmPrescaler", ImVec2(0, ImGui::GetFontSize() * 8.0f), true);
		static int prescalerPWM0_to_PWM3 = 0;
		static int prescalerPWM4_to_PWM7 = 0;
		if (ImGui::InputInt("Set prescaler from PWM0 to PMW3", &prescalerPWM0_to_PWM3)) {
			// Saturation check
			if (prescalerPWM0_to_PWM3 < 0)
				prescalerPWM0_to_PWM3 = 0;
			else if (prescalerPWM0_to_PWM3 > 0xFFFF)
				prescalerPWM0_to_PWM3 = 0xFFFF;
		}
		if (ImGui::InputInt("Set prescaler from PWM4 to PMW7", &prescalerPWM4_to_PWM7)) {
			// Saturation check
			if (prescalerPWM4_to_PWM7 < 0)
				prescalerPWM4_to_PWM7 = 0;
			else if (prescalerPWM4_to_PWM7 > 0xFFFF)
				prescalerPWM4_to_PWM7 = 0xFFFF;
		}
		if (ImGui::Button("Send prescalers")) {
			// Send the prescaler to STM32PLC
			setPWMPrescalerToSTM32PLC(0, prescalerPWM0_to_PWM3);
			setPWMPrescalerToSTM32PLC(1, prescalerPWM4_to_PWM7);
		}
		ImGui::SameLine();
		if (ImGui::Button("Recieve prescalers")) {
			// Clear the current prescalers
			resetPwmPrescalers();

			// Ask the prescaler to STM32PLC
			askPWMPrescalersFromSTM32();
		}
		uint16_t prescalersPWM[2] = { 0 };
		getPwmPrescalers(prescalersPWM);
		char text[100];
		sprintf_s(text, "Prescaler PWM0 to PWM3 = %i, PWM4 to PWM7 = %i", prescalersPWM[0], prescalersPWM[1]);
		ImGui::Text(text);
		ImGui::EndChild();

		// Gain
		ImGui::Text("Gain:");
		ImGui::BeginChild("analogGain", ImVec2(0, ImGui::GetFontSize() * 19.0f), true);
		static int analogPeripheralIndex = 1;
		static int analogConfigurationIndex = 0;
		static int analogGainIndex = 0;
		ImGui::Text("Peripheral 1 = ADC0 to ADC8");
		ImGui::Text("Peripheral 2 = ADC9 to ADC11");
		ImGui::Text("Peripheral 3 = DADC0 to DADC4");
		ImGui::Text("Configuration 0 = ADC0, ADC2, ADC9, DADC0, DADC1");
		ImGui::Text("Configuration 1 = ADC3, ADC4, ADC5, ADC10, DADC2, DADC3");
		ImGui::Text("Configuration 2 = ADC6, ADC7, ADC8, ADC11, DADC4");
		ImGui::Text("Gains: 0 = 1x, 1 = 2x, 2 = 4x, 3 = 8x, 4 = 16x, 5 = 32x, 7 = 0.5x");
		if (ImGui::InputInt("Select analog peripheral", &analogPeripheralIndex)) {
			// Saturation check
			if (analogPeripheralIndex < 1)
				analogPeripheralIndex = 1;
			else if (analogPeripheralIndex > 3)
				analogPeripheralIndex = 3;
		}
		if (ImGui::InputInt("Select analog configuration", &analogConfigurationIndex)) {
			// Saturation check
			if (analogConfigurationIndex < 0)
				analogConfigurationIndex = 0;
			else if (analogConfigurationIndex > 2)
				analogConfigurationIndex = 2;
		}
		if (ImGui::InputInt("Select analog gain", &analogGainIndex)) {
			// We cannot use number 6 according to the data sheet of the STM32
			static int pastAnalogGainIndex = 0;
			if (pastAnalogGainIndex == 5 && analogGainIndex == 6)
				analogGainIndex++;
			else if (pastAnalogGainIndex == 7 && analogGainIndex == 6)
				analogGainIndex--;
			
			// Saturation check
			if (analogGainIndex < 0)
				analogGainIndex = 0;
			else if (analogGainIndex > 7)
				analogGainIndex = 7;

			pastAnalogGainIndex = analogGainIndex;

		}
		if (ImGui::Button("Send gain")) {
			// Send the gain to STM32PLC
			setAnalogInputGainToSTM32PLC(analogPeripheralIndex, analogConfigurationIndex, analogGainIndex);
		}
		ImGui::SameLine();
		if (ImGui::Button("Recieve gain")) {
			// Clear the current gain
			resetAnalogGain();

			// Ask the prescaler to STM32PLC
			askAnalogInputGainsFromSTM32(analogPeripheralIndex);
		}
		uint8_t analogGain[3] = { 0 };
		getAnalogGain(analogGain);
		sprintf_s(text, "Gain for configuration 0 = %i, Gain for configuration 1 = %i, Gain for configuration 2 = %i", analogGain[0], analogGain[1], analogGain[2]);
		ImGui::Text(text);
		ImGui::EndChild();

		// Clock
		ImGui::Text("Clock:");
		ImGui::BeginChild("clock", ImVec2(0, ImGui::GetFontSize() * 13.0f), true);
		static int year = 2000;
		static int month = 1;
		static int date = 1;
		static int hour = 0;
		static int minute = 0;
		if (ImGui::InputInt("Year", &year)) {
			if (year < 2000)
				year = 2000;
			else if (year > 2100)
				year = 2100;
		}
		if (ImGui::InputInt("Month", &month)) {
			if (month < 1)
				month = 1;
			else if (month > 12)
				month = 12;
		}
		if (ImGui::InputInt("Date", &date)) {
			if (date < 1)
				date = 1;
			else if (date > 31)
				date = 31;
		}
		if (ImGui::InputInt("Hour", &hour)) {
			if (hour < 0)
				hour = 0;
			else if (hour > 23)
				hour = 23;
		}
		if (ImGui::InputInt("Minute", &minute)) {
			if (minute < 0)
				minute = 0;
			else if (minute > 59)
				minute = 59;
		}
		if (ImGui::Button("Send date time")) {
			// Send the date time to STM32PLC
			int date_p = date; // Save this because this is going to change
			int year_p = year;
			uint8_t weekDay = (date += month < 3 ? year-- : year - 2, 23 * month / 9 + date + 4 + year / 4 - year / 100 + year / 400) % 7;
			date = date_p;
			year = year_p;
			setDateTimeToSTM32PLC(year - 2000, month, date, weekDay, hour, minute);
		}
		ImGui::SameLine();
		if (ImGui::Button("Recieve date time")) {
			// Clear the current date time
			resetAnalogGain();

			// Ask the date time to STM32PLC
			askDateTimeFromSTM32PLC();
		}
		uint8_t year_, month_, date_, hour_, minute_;
		getDateTime(&year_, &month_, &date_, &hour_, &minute_);
		sprintf_s(text, "Date time: %i-%i-%i %i:%i", year_+2000, month_, date_, hour_, minute_);
		ImGui::Text(text);
		ImGui::EndChild();

		// Alarm A
		ImGui::Text("Alarm A:");
		ImGui::BeginChild("alarmA", ImVec2(0, ImGui::GetFontSize() * 12.0f), true);
		static bool enableAlarmA = false;
		static int dateAlarmA = 1;
		static int hourAlarmA = 0;
		static int minuteAlarmA = 0;
		ImGui::Checkbox("Enable", &enableAlarmA);
		if (ImGui::InputInt("Date", &dateAlarmA)) {
			if (dateAlarmA < 1)
				dateAlarmA = 1;
			else if (dateAlarmA > 31)
				dateAlarmA = 31;
		}
		if (ImGui::InputInt("Hour", &hourAlarmA)) {
			if (hourAlarmA < 0)
				hourAlarmA = 0;
			else if (hourAlarmA > 23)
				hourAlarmA = 23;
		}
		if (ImGui::InputInt("Minute", &minuteAlarmA)) {
			if (minuteAlarmA < 0)
				minuteAlarmA = 0;
			else if (minuteAlarmA > 59)
				minuteAlarmA = 59;
		}
		if (ImGui::Button("Send alarm A")) {
			// Send the alarm A to STM32PLC
			setAlarmAToSTM32PLC(dateAlarmA, hourAlarmA, minuteAlarmA, enableAlarmA);
		}
		ImGui::SameLine();
		if (ImGui::Button("Recieve alarm A")) {
			// Clear the current alarm A
			resetAlarmA();

			// Ask the alarm A to STM32PLC
			askAlarmAFromSTM32PLC();
		}
		uint8_t enabledAlarmA_, activatedAlarmA_, dateAlarmA_, hourAlarmA_, minuteAlarmA_;
		getAlarmA(&enabledAlarmA_, &activatedAlarmA_, &dateAlarmA_, &hourAlarmA_, &minuteAlarmA_);
		sprintf_s(text, "Alarm A(%s, %s): Date:%i Time:%i:%i", enabledAlarmA_ == 1 ? "enabled" : "disabled", activatedAlarmA_ == 1 ? "activated" : "deactivated", dateAlarmA_, hourAlarmA_, minuteAlarmA_);
		ImGui::Text(text);
		ImGui::EndChild();

		// Alarm B
		ImGui::Text("Alarm B:");
		ImGui::BeginChild("alarmB", ImVec2(0, ImGui::GetFontSize() * 12.0f), true);
		static bool enableAlarmB = false;
		static int weekDayAlarmB = 1;
		static int hourAlarmB = 0;
		static int minuteAlarmB = 0;
		ImGui::Checkbox("Enable", &enableAlarmB);
		if (ImGui::InputInt("Weekday", &weekDayAlarmB)) {
			if (weekDayAlarmB < 1)
				weekDayAlarmB = 1;
			else if (weekDayAlarmB > 7)
				weekDayAlarmB = 7;
		}
		if (ImGui::InputInt("Hour", &hourAlarmB)) {
			if (hourAlarmB < 0)
				hourAlarmB = 0;
			else if (hourAlarmB > 23)
				hourAlarmB = 23;
		}
		if (ImGui::InputInt("Minute", &minuteAlarmB)) {
			if (minuteAlarmB < 0)
				minuteAlarmB = 0;
			else if (minuteAlarmB > 59)
				minuteAlarmB = 59;
		}
		if (ImGui::Button("Send alarm B")) {
			// Send the alarm B to STM32PLC
			setAlarmBToSTM32PLC(weekDayAlarmB, hourAlarmB, minuteAlarmB, enableAlarmB);
		}
		ImGui::SameLine();
		if (ImGui::Button("Recieve alarm B")) {
			// Clear the current alarm B
			resetAlarmB();

			// Ask the alarm B to STM32PLC
			askAlarmBFromSTM32PLC();
		}
		uint8_t enabledAlarmB_, activatedAlarmB_, weekDayAlarmB_, hourAlarmB_, minuteAlarmB_;
		getAlarmB(&enabledAlarmB_, &activatedAlarmB_, &weekDayAlarmB_, &hourAlarmB_, &minuteAlarmB_);
		sprintf_s(text, "Alarm B(%s, %s): Weekday:%i Time:%i:%i", enabledAlarmB_ == 1 ? "enabled" : "disabled", activatedAlarmB_ == 1 ? "activated" : "deactivated", weekDayAlarmB_, hourAlarmB_, minuteAlarmB_);
		ImGui::Text(text);
		ImGui::EndChild();

	}
	else {
		ImGui::Text("You need to be connected to the USB.");
	}
	ImGui::End();
}