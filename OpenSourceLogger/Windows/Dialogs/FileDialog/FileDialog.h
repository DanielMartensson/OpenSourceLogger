#define _CRT_SECURE_NO_WARNINGS

#ifndef FileDialog
#define FileDialog

enum class FileDialogType {
	OpenFile,
	SelectFolder
};

void showFileDialog(bool* file_dialog_open, char* buffer, unsigned int buffer_size, FileDialogType type);

#endif // !FileDialog
