#include <iostream>
#include <cstring>
#include <string>
#include "pch.h"
using namespace std;

extern "C" {
	__declspec(dllexport) char* encrypt(char* rawText, int key) {
		char* encryptedText = new char[strlen(rawText) + 1];
		key = key % 26;
		for (int i = 0; i <= strlen(rawText); i++) {
			if (isupper(rawText[i])) {
				encryptedText[i] = char(int(rawText[i] + key - 65) % 26 + 65);
			}
			else if (islower(rawText[i])) {
				encryptedText[i] = char(int(rawText[i] + key - 97) % 26 + 97);
			}
			else {
				encryptedText[i] = rawText[i];
			}
		}
		encryptedText[strlen(rawText)] = '\0';
		return encryptedText;
	}

	__declspec(dllexport) char* decrypt(char* encryptedText, int key) {
		key = 26 - (key % 26);
		char* decryptedText = new char[strlen(encryptedText) + 1];
		for (int i = 0; i <= strlen(encryptedText); i++) {
			if (isupper(encryptedText[i])) {
				decryptedText[i] = char(int(encryptedText[i] + key - 65) % 26 + 65);
			}
			else if (islower(encryptedText[i])) {
				decryptedText[i] = char(int(encryptedText[i] + key - 97) % 26 + 97);
			}
			else {
				decryptedText[i] = encryptedText[i];
			}
		}
		decryptedText[strlen(encryptedText)] = '\0';
		return decryptedText;
	}
}
