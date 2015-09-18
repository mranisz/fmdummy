#include <cstdio>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include "common.h"
#include "sais.h"

using namespace std;

unsigned long long getFileSize(char* inFileName, int elemSize) {
	FILE* InFile;
	InFile = fopen(inFileName, "rb");
	if (InFile == NULL) {
		cout << "Can't open file " << inFileName << endl;
		exit(1);
	}
	if (fseek(InFile, 0, SEEK_END) != 0) {
		cout << "Something is wrong with file " << inFileName << endl;
		exit(1);
	}
	long long fileSize = ftell(InFile);
	if (fileSize == -1) {
		cout << "Something is wrong with file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return fileSize / elemSize;
}

FILE* openFile(char* inFileName, int elemSize, unsigned int &len) {
	FILE* InFile;
	InFile = fopen(inFileName, "rb");
	if (InFile == NULL) {
		cout << "Can't open file " << inFileName << endl;
		exit(1);
	}
	if (fseek(InFile, 0, SEEK_END) != 0) {
		cout << "Something is wrong with file " << inFileName << endl;
		exit(1);
	}
	long long fileSize = ftell(InFile);
	if (fileSize == -1) {
		cout << "Something is wrong with file " << inFileName << endl;
		exit(1);
	}
	if (fileSize == 0) {
		cout << "File " << inFileName << " is empty." << endl;
		exit(1);
	}
	rewind(InFile);
	len = fileSize / elemSize;
	return InFile;
}

unsigned char* readFileChar(char* inFileName, unsigned int &len, unsigned int addLen) {
	FILE* InFile = openFile(inFileName, 1, len);
	unsigned char* S = new unsigned char[len + addLen];
	if (fread(S, (size_t)1, (size_t)len, InFile) != (size_t)len) {
		cout << "Error reading file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return S;
}

unsigned int *readFileInt(char* inFileName, unsigned int &len, unsigned int addLen) {
	FILE* InFile = openFile(inFileName, 4, len);
	unsigned int* S = new unsigned int[len + addLen];
	if (fread(S, (size_t)4, (size_t)len, InFile) != (size_t)len) {
		cout << "Error reading file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return S;
}

unsigned long long *readFileLong(char* inFileName, unsigned int &len, unsigned int addLen) {
	FILE* InFile = openFile(inFileName, 8, len);
	unsigned long long* S = new unsigned long long[len + addLen];
	if (fread(S, (size_t)8, (size_t)len, InFile) != (size_t)len) {
		cout << "Error reading file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return S;
}

bool fileExists(char* inFileName) {
	FILE* InFile;
	InFile = fopen(inFileName, "rb");
	if (InFile == NULL) {
		return false;
	}
	if (InFile == NULL) {
		return false;
	}
	if (fseek(InFile, 0, SEEK_END) != 0) {
		return false;
	}
	long long fileSize = ftell(InFile);
	if (fileSize == -1) {
		return false;
	}
	if (fileSize == 0) {
		return false;
	}
	fclose(InFile);
	return true;
}

unsigned int *readSA(char* inFileName, unsigned int &len, unsigned int addLen, bool verbose) {
	unsigned int *sa;
	if (!fileExists((char *)((string)inFileName + (string)".sa").c_str())) {
		if (verbose) cout << "Creating SA for " << inFileName << " ... " << flush;
		unsigned int textLen;
		unsigned char *text = readFileChar(inFileName, textLen, 0);
		sa = new unsigned int[textLen + 1];
		sa[0] = textLen;
		++sa;
		sais(text, (int *)sa, textLen);
		if (verbose) cout << "Done" << endl;
		if (verbose) cout << "Saving SA in " << inFileName << ".sa ... " << flush;
		FILE* outFile;
		--sa;
		outFile = fopen((char *)((string)inFileName + (string)".sa").c_str(), "w");
		fwrite(sa, (size_t)4, (size_t)(textLen + 1), outFile);
		fclose(outFile);
		if (verbose) cout << "Done" << endl;
	} else {
		if (verbose) cout << "Reading SA from " << inFileName << ".sa ... " << flush;
		sa = readFileInt((char *)((string)inFileName + (string)".sa").c_str(), len, addLen);
		if (verbose) cout << "Done" << endl;
	}
	return sa;
}

unsigned char *readText(char* inFileName, unsigned int &len, unsigned char eof, bool checkNullChar) {
	unsigned char* S = readFileChar(inFileName, len, 1);
	S[len] = eof;
	if (checkNullChar) {
		for (unsigned int i = 0; i < len; ++i) {
			if (S[i] == '\0') {
				cout << "Error reading file: file contains at least one 0 character" << endl;
				exit(1);
			}
		}
	}
	return S;
}

unsigned char *getBwt(char *fileName, unsigned int &len, unsigned char eof, bool verbose) {

	unsigned int saLen;
	unsigned int *sa = readSA(fileName, saLen, 0, verbose);
	if (verbose) cout << "Getting BWT for " << fileName << " ... " << flush;
	unsigned int textLen;
	unsigned char *text = readFileChar(fileName, textLen, 0);
	len = textLen + 1;
	unsigned char *bwt = new unsigned char[len];
	bwt[0] = text[textLen - 1];

	for (unsigned int i = 1; i < saLen; ++i) {
		if (sa[i] == 0) bwt[i] = eof;
		else bwt[i] = text[sa[i] - 1];
	}
	delete[] text;
	delete[] sa;
	if (verbose) cout << "Done" << endl;
	return bwt;
}

unsigned int *breakByDelimeter(string seq, char delim, unsigned int &tokensLen) {
	char *s = (char *)seq.c_str();
	int c1 = 1;
	char *p = s;
	while (*p != '\0') {
		if (*p == delim) {
			++c1;
		}
		++p;
	}
	tokensLen = c1;
	unsigned int *res = new unsigned int[c1];
	char *token = new char[10];
	c1 = 0;
	int c2 = 0;
	p = s;
	while (*p != '\0') {
		if (*p == delim) {
			token[c1] = '\0';
			c1 = 0;
			res[c2++] = (unsigned int)atoi(token);
		}
		else {
			token[c1++] = *p;
		}
		++p;
	}
	token[c1] = '\0';
	res[c2] = (unsigned int)atoi(token);
	delete[] token;
	return res;
}
