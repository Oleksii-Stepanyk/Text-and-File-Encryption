#include <iostream>
#include <fstream>
#include <cstring>
#include <windows.h>
using namespace std;

typedef char* (*function)(char*, int);
class TextEditor;

class CaesarCipher {
private:
	function encrypt;
	function decrypt;
	HINSTANCE handle;

	int loadLibrary() {

		handle = LoadLibrary(TEXT("Cipher.dll")); if
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
	}

	int unloadLibrary() {
		FreeLibrary(handle);
	}

public:

	char* Encrypt(char* text, int key) {
		return encrypt(text, key);
	}

	char* Decrypt(char* text, int key) {
		return decrypt(text, key);
	}
};

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
	char** text;
	int rows;
	int cols;
	int total_rows;

	Text() {
		rows = 10;
		cols = 128;
		total_rows = 0;
		allocate_array(rows, cols);
	}

	Text(const Text& other) {
		rows = other.rows;
		cols = other.cols;
		total_rows = other.total_rows;
		allocate_array(rows, cols);
		for (int i = 0; i <= total_rows; i++) {
			strcpy_s(text[i], cols, other.text[i]);
		}
	}

	~Text() {
		deallocate_array();
	}

	void allocate_array(int& rows, int& cols) {
		text = (char**)malloc(rows * sizeof(char*));
		if (text == NULL) {
			cerr << "Failed to allocate memory";
			return;
		}
		for (int i = 0; i < rows; i++) {
			text[i] = (char*)malloc(cols * sizeof(char));
			if (text[i] == NULL) {
				cerr << "Failed to allocate memory";
				return;
			}
			text[i][0] = '\0';
		}
	}

    void reallocate_rows(int& new_rows) {
        char** new_text = (char**)realloc(text, new_rows * sizeof(char*));
        if (new_text == NULL) {
            cerr << "Failed to allocate memory";
            return;
        }
        text = new_text;
        for (int i = rows; i < new_rows; i++) {
            text[i] = (char*)malloc(cols * sizeof(char));
            if (text[i] == NULL) {
                cerr << "Failed to allocate memory";
                return;
            }
        }
        rows = new_rows;
    }

    void reallocate_cols(int& new_cols) {
        for (int i = 0; i < rows; i++) {
            char* new_row = (char*)realloc(text[i], new_cols * sizeof(char));
            if (new_row == NULL) {
                cerr << "Failed to allocate memory";
                return;
            }
            text[i] = new_row;
        }
        cols = new_cols;
    }

	void deallocate_array() {
		if (text != nullptr) {
			for (int i = 0; i < rows; i++) {
				if (text[i] != nullptr) {
					free(text[i]);
				}
			}
			free(text);
			text = nullptr;
		}
	}

	void shift_right(int& row, int& start_index, int shift_amount) {
		if (start_index + shift_amount >= cols) {
			int new_cols = cols + 128;
			reallocate_cols(new_cols);
		}
		for (int i = cols; i >= start_index; i--) {
			text[row][i + shift_amount] = text[row][i];
		}
		text[row][strlen(text[row]) + 1] = '\0';
	}
};

class Text_Buffer {
public:
	Text text_storage;
	Cursor cursor;
	char* paste_buffer;

	Text_Buffer(Text* text, Cursor* curs, char* pasted_buffer) {
		text_storage = *text;
		cursor = *curs;
		paste_buffer = new char[256];
		strcpy_s(paste_buffer, 256, pasted_buffer);
	}

	~Text_Buffer() {
		delete[] paste_buffer;
	}
};

class TextEditor {
private:
	Text* text_storage;
	char* paste_buffer;
	Text_Buffer* undo1 = nullptr;
	Text_Buffer* undo2 = nullptr;
	Text_Buffer* undo3 = nullptr;
	Text_Buffer* redo1 = nullptr;
	Text_Buffer* redo2 = nullptr;
	Text_Buffer* redo3 = nullptr;

	void add_undo(Cursor* cursor) {
		if (undo3) delete undo3;
		undo3 = undo2;
		undo2 = undo1;
		undo1 = new Text_Buffer(text_storage, cursor, paste_buffer);
	}

	void add_redo(Cursor* cursor) {
		if (redo3) delete redo3;
		redo3 = redo2;
		redo2 = redo1;
		redo1 = new Text_Buffer(text_storage, cursor, paste_buffer);
	}

	void restore_buffer(Cursor* cursor, Text_Buffer*& buf_1) {
		*cursor = buf_1->cursor;
		*text_storage = buf_1->text_storage;
		strcpy_s(paste_buffer, 256, buf_1->paste_buffer);

		delete buf_1;
		buf_1 = nullptr;
	}

	int get_input(Cursor* cursor, string action, int& row, int& col, int& length) {
		row = cursor->row;
		col = cursor->col;
		cout << "Enter the length of text to " << action << " : ";
		cin >> length;
		if (col + length >= strlen(text_storage->text[row])) {
			cerr << "The length is out of range" << endl;
			return -1;
		}
	}

public:
	TextEditor() {
		text_storage = new Text;
		paste_buffer = new char[256];
		paste_buffer[0] = '\0';
	}

	~TextEditor() {
		delete text_storage;
		delete[] paste_buffer;

		if (undo1) delete undo1;
		if (undo2) delete undo2;
		if (undo3) delete undo3;
		if (redo1) delete redo1;
		if (redo2) delete redo2;
		if (redo3) delete redo3;
	}

	char** get_text() {
		return text_storage->text;
	}

	int get_rows() {
		return text_storage->total_rows;
	}

	void print_help() {
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
			<< "17: Command list" << endl
			<< "0: Exit program" << endl;
	}

	void append_text(Cursor* cursor) {
		char buffer[256];
		cursor->_SystemMoveCursor(text_storage->total_rows, (int)strlen(text_storage->text[text_storage->total_rows]));
		char* position = cursor->GetPosition(text_storage->text);

		cout << "Enter text to append: " << endl;
		cin.ignore();
		cin.getline(buffer, 256);
		buffer[cin.gcount()] = '\0';

		if (strlen(text_storage->text[text_storage->total_rows]) + strlen(buffer) >= text_storage->cols) {
			int new_cols = text_storage->cols + 128;
			text_storage->reallocate_cols(new_cols);
			position = cursor->GetPosition(text_storage->text);
			position = strstr(position, "\0");
		}
		add_undo(cursor);
		strncat_s(position, (int)strlen(buffer) + 1, (const char*)&buffer, _TRUNCATE);
		int total_cols = (int)strlen(text_storage->text[text_storage->total_rows]);
		cursor->_SystemMoveCursor(cursor->row, total_cols);
	}

	void start_newline(Cursor* cursor) {
		add_undo(cursor);
		char* position = cursor->GetPosition(text_storage->text);
		*position = '\n';
		position++;
		*position = '\0';
		if (cursor->row + 1 >= text_storage->rows) {
			int new_rows = text_storage->rows + 10;
			text_storage->reallocate_rows(new_rows);
		}
		(text_storage->total_rows)++;
		cursor->_SystemMoveCursor(cursor->row + 1, 0);
		*cursor->GetPosition(text_storage->text) = '\0';
	}

	void save_file() {
		char file_name[32];
		cout << "Enter the file name for saving: ";
		cin >> file_name;
		ofstream file(file_name);
		if (!file.fail()) {
			for (int i = 0; i <= text_storage->total_rows; i++) {
				file << text_storage->text[i];
			}
			file.close();
			cout << "Text saved successfully" << endl;
		}
	}

	void load_file(Cursor* cursor)
	{
		char file_name[32];
		char ch;
		int rows = 0;
		cursor->_SystemMoveCursor(0, 0);
		char* position = cursor->GetPosition(text_storage->text);
		cout << "Enter the file name for loading: ";
		cin >> file_name;
		ifstream file(file_name);
		if (!file.is_open()) {
			cerr << "Error opening file" << endl;
			return;
		}
		while (file.get(ch)) {
			if (strlen(text_storage->text[rows]) + 1 >= text_storage->cols) {
				int new_cols = text_storage->cols + 128;
				text_storage->reallocate_cols(new_cols);
				position = cursor->GetPosition(text_storage->text);
			}
			if (ch != '\n') {
				*position = ch;
				position++;
				*position = '\0';
				cursor->_SystemMoveCursor(rows, cursor->col + 1);
			}
			else if (ch == '\n') {
				if (text_storage->total_rows + 1 >= text_storage->rows) {
					int new_rows = text_storage->rows + 10;
					text_storage->reallocate_rows(new_rows);
					position = cursor->GetPosition(text_storage->text);
				}
				cursor->_SystemMoveCursor(rows, cursor->col);
				start_newline(cursor);
				rows++;
				position = cursor->GetPosition(text_storage->text);
			}
		}
		file.close();
		cout << "Text loaded successfully" << endl;
		cursor->_SystemMoveCursor(rows, strlen(text_storage->text[rows]));
	}

	void print_text() {
		for (int i = 0; i <= text_storage->total_rows; i++) {
			cout << text_storage->text[i];
		}
		cout << endl;
	}

	void insert_text(Cursor* cursor) {
		int row = cursor->row;
		int col = cursor->col;
		char entered_text[64];
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		add_undo(cursor);
		text_storage->shift_right(row, col, (int)strlen(entered_text));
		for (int i = 0; i < (int)strlen(entered_text); i++) {
			text_storage->text[row][col + i] = entered_text[i];
		}
	}

	void search_text() {
		char search_text[64];
		char* position;
		bool found = false;
		int index;
		cout << "Enter text you want to find:";
		cin.ignore();
		cin.getline(search_text, 64);
		search_text[cin.gcount()] = '\0';
		cout << "Text found in: ";
		for (int i = 0; i <= text_storage->total_rows; ++i) {
			position = text_storage->text[i];
			while ((position = strstr(position, search_text)) != NULL) {
				index = (int)(position - text_storage->text[i]);
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

	void delete_text(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		get_input(cursor, "delete", row, col, length);
		if (row == -1) return;
		add_undo(cursor);
		for (int i = col; i < strlen(text_storage->text[row]) - length; i++) {
			text_storage->text[row][i] = text_storage->text[row][i + length];
		}
		text_storage->text[row][strlen(text_storage->text[row]) - length] = '\0';
		cout << "Text deleted successfully" << endl;
	}

	void cut_text(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		get_input(cursor, "cut", row, col, length);
		if (row == -1) return;
		add_undo(cursor);
		for (int i = col; i < col + length; i++) {
			paste_buffer[i - col] = text_storage->text[row][i];
		}
		paste_buffer[length] = '\0';

		for (int i = col; i <= strlen(text_storage->text[row]) - length; i++) {
			text_storage->text[row][i] = text_storage->text[row][i + length];
		}
		cout << "Text cut successfully" << endl;
	}

	void copy_text(Cursor* cursor) {
		int row = 0, col = 0, length = 0;
		get_input(cursor, "copy", row, col, length);
		if (row == -1) return;
		add_undo(cursor);
		for (int i = col; i < col + length; i++) {
			paste_buffer[i - col] = text_storage->text[row][i];
		}
		paste_buffer[length] = '\0';
		cout << "Text copied successfully" << endl;
	}

	void paste_text(Cursor* cursor) {
		if (paste_buffer[0] == '\0') {
			cerr << "The buffer is empty" << endl;
			return;
		}
		int row = cursor->row;
		int col = cursor->col;
		if (col + strlen(paste_buffer) >= text_storage->cols - 1) {
			cerr << "The length is out of range" << endl;
			return;
		}
		add_undo(cursor);
		text_storage->shift_right(row, col, (int)strlen(paste_buffer));
		for (int i = 0; i < (int)strlen(paste_buffer); i++) {
			text_storage->text[row][col + i] = paste_buffer[i];
		}
		cout << "Text pasted successfully" << endl;
	}

	void insert_and_replace(Cursor* cursor) {
		int row = cursor->row;
		int col = cursor->col;
		char entered_text[64];
		cout << "Enter text to insert: ";
		cin.ignore();
		cin.getline(entered_text, 256);
		entered_text[cin.gcount()] = '\0';
		add_undo(cursor);
		for (int i = 0; i < (int)strlen(entered_text); i++) {
			text_storage->text[row][col + i] = entered_text[i];
		}
	}

	void undo_command(Cursor* cursor) {
		if (undo1) {
			add_redo(cursor);
			restore_buffer(cursor, undo1);
			undo1 = undo2;
			undo2 = undo3;
			undo3 = nullptr;
		}
		else {
			cerr << "Nothing to undo" << endl;
		}
	}

	void redo_command(Cursor* cursor) {
		if (redo1) {
			restore_buffer(cursor, redo1);
			redo1 = redo2;
			redo2 = redo3;
			redo3 = nullptr;
		}
		else {
			cerr << "Nothing to redo" << endl;
		}
	}

	void clear_console() {
#ifdef _WIN64
		system("cls");
#else
		system("clear");
#endif
	}
};

void Cursor::MoveCursor(TextEditor* editor) {
	int new_row, new_col;
	int total_rows = editor->get_rows();
	char** text = editor->get_text();
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

int main() {
	TextEditor* editor = new TextEditor();
	Cursor* cursor = new Cursor();
	while (true) {
		int input;
		cout << "Choose the command or enter 17 for commands list:" << endl;
		cin >> input;
		switch (input) {
		case 1:
			editor->append_text(cursor);
			break;
		case 2:
			editor->start_newline(cursor);
			break;
		case 3:
			editor->save_file();
			break;
		case 4:
			editor->load_file(cursor);
			break;
		case 5:
			editor->print_text();
			break;
		case 6:
			editor->insert_text(cursor);
			break;
		case 7:
			editor->search_text();
			break;
		case 8:
			editor->delete_text(cursor);
			break;
		case 9:
			editor->undo_command(cursor);
			break;
		case 10:
			editor->redo_command(cursor);
			break;
		case 11:
			editor->cut_text(cursor);
			break;
		case 12:
			editor->copy_text(cursor);
			break;
		case 13:
			editor->paste_text(cursor);
			break;
		case 14:
			editor->insert_and_replace(cursor);
			break;
		case 15:
			cursor->MoveCursor(editor);
			break;
		case 16:
			editor->clear_console();
			break;
		case 17:
			editor->print_help();
			break;
		case 0:
			delete editor;
			delete cursor;
			return 0;
		default:
			cout << "The command is not implemented" << endl;
			break;
		}
	}
}