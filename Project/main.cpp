#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>
using namespace std;

typedef char* (*function)(char*, int);
class TextEditor;

class Cursor {
public:
	int row, col;

	Cursor() {
		row = 0;
		col = 0;
	}

	Cursor(const Cursor& other) {
		row = other.row;
		col = other.col;
	}

	void MoveCursor(TextEditor* editor);

	void _SystemMoveCursor(int new_row, int new_col) {
		row = new_row;
		col = new_col;
	}

	char* GetPosition(char** text) {
		return text[row] + col;
	}
};

class Text {

public:
	char** storage;
	int rows;
	int cols;
	int total_rows;

	Text() {
		rows = 10;
		cols = 128;
		total_rows = 0;
		AllocateArray(rows, cols);
	}

	Text(const Text& other) {
		rows = other.rows;
		cols = other.cols;
		total_rows = other.total_rows;
		AllocateArray(rows, cols);
		for (int i = 0; i <= total_rows; i++) {
			strcpy_s(storage[i], cols, other.storage[i]);
		}
	}

	~Text() {
		DeallocateArray();
	}

	void AllocateArray(int& rows, int& cols) {
		storage = (char**)malloc(rows * sizeof(char*));
		if (storage == NULL) {
			cerr << "Failed to allocate memory";
			return;
		}
		for (int i = 0; i < rows; i++) {
			storage[i] = (char*)malloc(cols * sizeof(char));
			if (storage[i] == NULL) {
				cerr << "Failed to allocate memory";
				return;
			}
			storage[i][0] = '\0';
		}
	}

	void ReallocateRows(int& new_rows) {
		char** new_text = (char**)realloc(storage, new_rows * sizeof(char*));
		if (new_text == NULL) {
			cerr << "Failed to allocate memory";
			return;
		}
		storage = new_text;
		for (int i = rows; i < new_rows; i++) {
			storage[i] = (char*)malloc(cols * sizeof(char));
			if (storage[i] == NULL) {
				cerr << "Failed to allocate memory";
				return;
			}
		}
		rows = new_rows;
	}

	void ReallocateCols(int& new_cols) {
		for (int i = 0; i < rows; i++) {
			char* new_row = (char*)realloc(storage[i], new_cols * sizeof(char));
			if (new_row == NULL) {
				cerr << "Failed to allocate memory";
				return;
			}
			storage[i] = new_row;
		}
		cols = new_cols;
	}

	void DeallocateArray() {
		if (storage != nullptr) {
			for (int i = 0; i < rows; i++) {
				if (storage[i] != nullptr) {
					free(storage[i]);
				}
			}
			free(storage);
			storage = nullptr;
		}
	}

	void ShiftRight(int& row, int& start_index, int shift_amount) {
		if (start_index + shift_amount >= cols) {
			int new_cols = cols + 128;
			ReallocateCols(new_cols);
		}
		for (int i = cols; i >= start_index; i--) {
			storage[row][i + shift_amount] = storage[row][i];
		}
		storage[row][strlen(storage[row]) + 1] = '\0';
	}
};

class Text_Buffer {
public:
	Text* text_storage;
	Cursor* cursor;
	char* paste_buffer;

	Text_Buffer(Text* text, Cursor* curs, char* pasted_buffer) {
		text_storage = new Text(*text);
		cursor = new Cursor(*curs);
		paste_buffer = new char[256];
		strcpy_s(paste_buffer, 256, pasted_buffer);
	}

	~Text_Buffer() {
		delete[] paste_buffer;
		delete cursor;
		delete text_storage;
	}
};

class TextEditor {
private:
	Text* text;
	char* paste_buffer;
	Text_Buffer* undo1 = nullptr;
	Text_Buffer* undo2 = nullptr;
	Text_Buffer* undo3 = nullptr;
	Text_Buffer* redo1 = nullptr;
	Text_Buffer* redo2 = nullptr;
	Text_Buffer* redo3 = nullptr;

	void _AddUndo(Cursor* cursor) {
		if (undo3) delete undo3;
		undo3 = undo2;
		undo2 = undo1;
		undo1 = new Text_Buffer(text, cursor, paste_buffer);
	}

	void _AddRedo(Cursor* cursor) {
		if (redo3) delete redo3;
		redo3 = redo2;
		redo2 = redo1;
		redo1 = new Text_Buffer(text, cursor, paste_buffer);
	}

	void _RestoreBuffer(Cursor* cursor, Text_Buffer*& buf_1) {
		cursor = new Cursor(*buf_1->cursor);
		text = new Text(*buf_1->text_storage);
		strcpy_s(paste_buffer, 256, buf_1->paste_buffer);

		delete buf_1;
		buf_1 = nullptr;
	}

	int _GetInput(Cursor* cursor, string action, int& row, int& col, int& length) {
		row = cursor->row;
		col = cursor->col;
		cout << "Enter the length of text to " << action << " : ";
		cin >> length;
		if (col + length >= strlen(text->storage[row])) {
			cerr << "The length is out of range" << endl;
			return -1;
		}
	}

public:
	TextEditor() {
		text = new Text;
		paste_buffer = new char[256];
		paste_buffer[0] = '\0';
	}

	~TextEditor() {
		delete text;
		delete[] paste_buffer;

		if (undo1) delete undo1;
		if (undo2) delete undo2;
		if (undo3) delete undo3;
		if (redo1) delete redo1;
		if (redo2) delete redo2;
		if (redo3) delete redo3;
	}

	char** GetText() {
		return text->storage;
	}

	int GetRows() {
		return text->total_rows;
	}

	void PrintHelp() {
		cout << "Commands: " << endl
			<< "1: Append text symbols to the end" << endl
			<< "2: Start the new line" << endl
			<< "3: Save text into file" << endl
			<< "4: Load text from file" << endl
			<< "5: Print the current text" << endl
			<< "6: Insert text by line and index" << endl
			<< "7: Search text placement" << endl
			<< "8: Delete text" << endl
			<< "9: Undo command" << endl
			<< "10: Redo command" << endl
			<< "11: Cut text" << endl
			<< "12: Copy text" << endl
			<< "13: Paste text" << endl
			<< "14: Insert text with replacement" << endl
			<< "15: Move cursor" << endl
			<< "16: Clear console" << endl
			<< "17: Encryption/Decryption" << endl
			<< "18: Command list" << endl
			<< "0: Exit program" << endl;
	}

	void AppendText(Cursor* cursor) {
		char buffer[256];
		cursor->_SystemMoveCursor(text->total_rows, (int)strlen(text->storage[text->total_rows]));
		char* position = cursor->GetPosition(text->storage);

		cout << "Enter text to append: " << endl;
		cin.ignore();
		cin.getline(buffer, 256);
		buffer[cin.gcount()] = '\0';

		if (strlen(text->storage[text->total_rows]) + strlen(buffer) >= text->cols) {
			int new_cols = text->cols + 128;
			text->ReallocateCols(new_cols);
			position = cursor->GetPosition(text->storage);
			position = strstr(position, "\0");
		}
		_AddUndo(cursor);
		strncat_s(position, (int)strlen(buffer) + 1, (const char*)&buffer, _TRUNCATE);
		int total_cols = (int)strlen(text->storage[text->total_rows]);
		cursor->_SystemMoveCursor(cursor->row, total_cols);
	}

	void StartNewline(Cursor* cursor) {
		_AddUndo(cursor);
		char* position = cursor->GetPosition(text->storage);
		*position = '\n';
		position++;
		*position = '\0';
		if (cursor->row + 1 >= text->rows) {
			int new_rows = text->rows + 10;
			text->ReallocateRows(new_rows);
		}
		(text->total_rows)++;
		cursor->_SystemMoveCursor(cursor->row + 1, 0);
		*cursor->GetPosition(text->storage) = '\0';
	}

	void SaveFile() {
		char file_name[32];
		cout << "Enter the file name for saving: ";
		cin >> file_name;
		ofstream file(file_name);
		if (!file.fail()) {
			for (int i = 0; i <= text->total_rows; i++) {
				file << text->storage[i];
			}
			file.close();
			cout << "Text saved successfully" << endl;
		}
	}

	void LoadFile(Cursor* cursor)
	{
		char file_name[32];
		char ch;
		int rows = 0;
		cursor->_SystemMoveCursor(0, 0);
		char* position = cursor->GetPosition(text->storage);
		cout << "Enter the file name for loading: ";
		cin >> file_name;
		ifstream file(file_name);
		if (!file.is_open()) {
			cerr << "Error opening file" << endl;
			return;
		}
		while (file.get(ch)) {
			if (strlen(text->storage[rows]) + 1 >= text->cols) {
				int new_cols = text->cols + 128;
				text->ReallocateCols(new_cols);
				position = cursor->GetPosition(text->storage);
			}
			if (ch != '\n') {
				*position = ch;
				position++;
				*position = '\0';
				cursor->_SystemMoveCursor(rows, cursor->col + 1);
			}
			else if (ch == '\n') {
				if (text->total_rows + 1 >= text->rows) {
					int new_rows = text->rows + 10;
					text->ReallocateRows(new_rows);
					position = cursor->GetPosition(text->storage);
				}
				cursor->_SystemMoveCursor(rows, cursor->col);
				StartNewline(cursor);
				rows++;
				position = cursor->GetPosition(text->storage);
			}
		}
		file.close();
		cout << "Text loaded successfully" << endl;
		cursor->_SystemMoveCursor(rows, strlen(text->storage[rows]));
	}

	void PrintText() {
		for (int i = 0; i <= text->total_rows; i++) {
			cout << text->storage[i];
		}
		cout << endl;
	}

	void InsertText(Cursor* cursor) {
		int row = cursor->row;
		int col = cursor->col;
		char entered_text[64];
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		_AddUndo(cursor);
		text->ShiftRight(row, col, (int)strlen(entered_text));
		for (int i = 0; i < (int)strlen(entered_text); i++) {
			text->storage[row][col + i] = entered_text[i];
		}
	}

	void SearchText() {
		char search_text[64];
		char* position;
		bool found = false;
		int index;
		cout << "Enter text you want to find:";
		cin.ignore();
		cin.getline(search_text, 64);
		search_text[cin.gcount()] = '\0';
		cout << "Text found in: ";
		for (int i = 0; i <= text->total_rows; ++i) {
			position = text->storage[i];
			while ((position = strstr(position, search_text)) != NULL) {
				index = (int)(position - text->storage[i]);
				cout << i << " " << index << "; ";
				position += strlen(search_text);
				found = true;
			}
		}
		cout << endl;
		if (!found) {
			cout << "Text not found" << endl;
		}
	}

	void DeleteText(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		_GetInput(cursor, "delete", row, col, length);
		if (row == -1) return;
		_AddUndo(cursor);
		for (int i = col; i < strlen(text->storage[row]) - length; i++) {
			text->storage[row][i] = text->storage[row][i + length];
		}
		text->storage[row][strlen(text->storage[row]) - length] = '\0';
		cout << "Text deleted successfully" << endl;
	}

	void CutText(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		_GetInput(cursor, "cut", row, col, length);
		if (row == -1) return;
		_AddUndo(cursor);
		for (int i = col; i < col + length; i++) {
			paste_buffer[i - col] = text->storage[row][i];
		}
		paste_buffer[length] = '\0';

		for (int i = col; i <= strlen(text->storage[row]) - length; i++) {
			text->storage[row][i] = text->storage[row][i + length];
		}
		cout << "Text cut successfully" << endl;
	}

	void CopyText(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		_GetInput(cursor, "copy", row, col, length);
		if (row == -1) return;
		_AddUndo(cursor);
		for (int i = col; i < col + length; i++) {
			paste_buffer[i - col] = text->storage[row][i];
		}
		paste_buffer[length] = '\0';
		cout << "Text copied successfully" << endl;
	}

	void PasteText(Cursor* cursor) {
		if (paste_buffer[0] == '\0') {
			cerr << "The buffer is empty" << endl;
			return;
		}
		int row = cursor->row;
		int col = cursor->col;
		if (col + strlen(paste_buffer) >= text->cols - 1) {
			cerr << "The length is out of range" << endl;
			return;
		}
		_AddUndo(cursor);
		text->ShiftRight(row, col, (int)strlen(paste_buffer));
		for (int i = 0; i < (int)strlen(paste_buffer); i++) {
			text->storage[row][col + i] = paste_buffer[i];
		}
		cout << "Text pasted successfully" << endl;
	}

	void InsertAndReplace(Cursor* cursor) {
		int row = cursor->row;
		int col = cursor->col;
		char entered_text[64];
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		_AddUndo(cursor);
		for (int i = 0; i < (int)strlen(entered_text); i++) {
			text->storage[row][col + i] = entered_text[i];
		}
	}

	void Undo(Cursor* cursor) {
		if (undo1) {
			_AddRedo(cursor);
			_RestoreBuffer(cursor, undo1);
			undo1 = undo2;
			undo2 = undo3;
			undo3 = nullptr;
		}
		else {
			cerr << "Nothing to undo" << endl;
		}
	}

	void Redo(Cursor* cursor) {
		if (redo1) {
			_RestoreBuffer(cursor, redo1);
			redo1 = redo2;
			redo2 = redo3;
			redo3 = nullptr;
		}
		else {
			cerr << "Nothing to redo" << endl;
		}
	}

	void ClearConsole() {
#ifdef _WIN64
		system("cls");
#else
		system("clear");
#endif
	}
};

void Cursor::MoveCursor(TextEditor* editor) {
	int new_row, new_col;
	int total_rows = editor->GetRows();
	char** text = editor->GetText();
	cout << "Enter the row and column to move cursor: ";
	cin >> new_row >> new_col;
	if (new_row > total_rows) {
		cout << "Row is out of range" << endl;
		return;
	}
	if (new_col > strlen(text[new_row])) {
		cout << "Column is out of range" << endl;
		return;
	}
	Cursor::row = new_row;
	Cursor::col = new_col;
}

class CaesarCipher {
private:
	function encrypt;
	function decrypt;
	HINSTANCE handle;

	int _LoadLibrary() {

		handle = LoadLibrary(TEXT("CaesarDLL.dll")); if
			(handle == nullptr || handle == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();
			return err;
		}
		encrypt = (function)GetProcAddress(handle, "encrypt");
		if (encrypt == nullptr)
		{
			cout << "Function not found" << endl;
			return 1;
		}
		decrypt = (function)GetProcAddress(handle, "decrypt");
		if (decrypt == nullptr)
		{
			cout << "Function not found" << endl;
			return 1;
		}
		return 0;
	}

	void _UnloadLibrary() {
		FreeLibrary(handle);
		handle = nullptr;
	}

public:

	void EncryptDecryptText(TextEditor* editor, function func, int key) {
		_LoadLibrary();
		char** text = editor->GetText();
		for (int i = 0; i <= editor->GetRows(); i++) {
			text[i] = func(text[i], key);
		}
		_UnloadLibrary();
	}

	void EncryptDecryptFile(function func, int key) {
		char fileRead[32];
		char fileWrite[32];
		char buffer[256];
		cout << "Enter the file name for reading: ";
		cin >> fileRead;
		cout << "Enter the file name for writing: ";
		cin >> fileWrite;
		ifstream file_R(fileRead);
		ofstream temp_W("temp.txt");
		if (!file_R.fail() && !temp_W.fail()) {
			while (file_R.getline(buffer, 256)) {
				temp_W << func(buffer, key) << endl;
			}
			file_R.close();
			temp_W.close();
		}
		ifstream temp_R("temp.txt");
		ofstream file_W(fileWrite);
		if (!temp_R.fail() && !file_W.fail()) {
			while (temp_R.getline(buffer, 256)) {
				file_W << buffer << endl;
			}
			temp_R.close();
			file_W.close();
		}
		remove("temp.txt");
		_UnloadLibrary();
	}

	void GetCommand(TextEditor* editor) {
		int command;
		int key;
		int err;
		cout << "1. Encrypt" << endl;
		cout << "2. Decrypt" << endl;
		cout << "3. Encrypt file" << endl;
		cout << "4. Decrypt file" << endl;
		cout << "Enter command: " << endl;
		cin >> command;
		cout << "Enter key: " << endl;
		cin >> key;
		err = _LoadLibrary();
		if (err != 0) return;
		switch (command) {
		case 1:
			EncryptDecryptText(editor, encrypt, key);
			break;
		case 2:
			EncryptDecryptText(editor, decrypt, key);
			break;
		case 3:
			EncryptDecryptFile(encrypt, key);
			break;
		case 4:
			EncryptDecryptFile(decrypt, key);
			break;
		default:
			cout << "Command not found" << endl;
			_UnloadLibrary();
			break;
		}
	}
};

int main() {
	TextEditor* editor = new TextEditor();
	Cursor* cursor = new Cursor();
	CaesarCipher* cipher = new CaesarCipher();
	while (true) {
		int input;
		cout << "Choose the command or enter 18 for commands list:" << endl;
		cin >> input;
		switch (input) {
		case 1:
			editor->AppendText(cursor);
			break;
		case 2:
			editor->StartNewline(cursor);
			break;
		case 3:
			editor->SaveFile();
			break;
		case 4:
			editor->LoadFile(cursor);
			break;
		case 5:
			editor->PrintText();
			break;
		case 6:
			editor->InsertText(cursor);
			break;
		case 7:
			editor->SearchText();
			break;
		case 8:
			editor->DeleteText(cursor);
			break;
		case 9:
			editor->Undo(cursor);
			break;
		case 10:
			editor->Redo(cursor);
			break;
		case 11:
			editor->CutText(cursor);
			break;
		case 12:
			editor->CopyText(cursor);
			break;
		case 13:
			editor->PasteText(cursor);
			break;
		case 14:
			editor->InsertAndReplace(cursor);
			break;
		case 15:
			cursor->MoveCursor(editor);
			break;
		case 16:
			editor->ClearConsole();
			break;
		case 17:
			cipher->GetCommand(editor);
			break;
		case 18:
			editor->PrintHelp();
			break;
		case 0:
			delete editor;
			delete cursor;
			delete cipher;
			return 0;
		default:
			cout << "The command is not implemented" << endl;
			break;
		}
	}
}