
#include "USBHandler.h"

// USB port
boost::asio::io_service usbPort;
boost::asio::streambuf readData;
boost::asio::serial_port port(usbPort);

bool openUSBConnection(std::string portName, int baudrateIndex, J1939* j1939) {
	// Connect 
	if (portName.length() > 0 && !isConnectedToUSB()) {
		// Open port
		port.open(portName);

		// Sett settings
		int baudrate = 0;
		switch (baudrateIndex) {
			case 0:baudrate = BAUD_110; break;
			case 1:baudrate = BAUD_150; break;
			case 2:baudrate = BAUD_300; break;
			case 3:baudrate = BAUD_600; break;
			case 4:baudrate = BAUD_1200; break;
			case 5:baudrate = BAUD_1800; break;
			case 6:baudrate = BAUD_2400; break;
			case 7:baudrate = BAUD_4800; break;
			case 8:baudrate = BAUD_9600; break;
			case 9:baudrate = BAUD_19200; break;
			case 10:baudrate = BAUD_38400; break;
			case 11:baudrate = BAUD_57600; break;
			case 12:baudrate = BAUD_115200; break;
		}
		port.set_option(boost::asio::serial_port_base::baud_rate(baudrate));

		// Start thread
		setReadUSBThreadStart(true);

		// Set can-bus call back functions
		activateJ1939CallBackAndSetJ1939AndSetUSBPort(j1939, &port);

		// Now return
		return isConnectedToUSB();
	}
	else {
		return false;
	}
}

bool closeUSBConnection() {
	setReadUSBThreadStart(false);
	if (isConnectedToUSB()) {
		port.close();
	}
	return !isConnectedToUSB();
}

bool isConnectedToUSB() {
	return port.is_open();
}

inline void checkPort(std::vector<std::string>& portNames, char portName[]) {
	port.open(portName);
	if (port.is_open()) {
		portNames.push_back(portName);
	}
	port.close();
}

void getUSBPortNames(std::vector<std::string>& portNames) {
	// Get the names
	char portName[15];
	for (int i = 0; i < 256; i++) {
		try {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
			sprintf_s(portName, "COM%i", i);
			checkPort(portNames, portName);
#else
			sprintf_s(portName, "/dev/ttyAMC%i", i);
			checkPort(portNames, portName);
			sprintf_s(portName, "/dev/ttyUSB%i", i);
			checkPort(portNames, portName);
#endif

		}
		catch (...) {}
	}
}

void closeUSBThread() {
	closeUSBConnection();
	setUSBThreadActive(false);
}