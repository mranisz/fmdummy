#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include "common.h"
#include "sais.h"

unsigned long long getFileSize(char *inFileName, int elemSize) {
	FILE *InFile;
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

FILE *openFile(char *inFileName, int elemSize, unsigned int &len) {
	FILE *InFile;
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

unsigned char *readFileChar(char *inFileName, unsigned int &len, unsigned int addLen) {
	FILE *InFile = openFile(inFileName, 1, len);
	unsigned char *S = new unsigned char[len + addLen];
	if (fread(S, (size_t)1, (size_t)len, InFile) != (size_t)len) {
		cout << "Error reading file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return S;
}

unsigned int *readFileInt(char *inFileName, unsigned int &len, unsigned int addLen) {
	FILE *InFile = openFile(inFileName, 4, len);
	unsigned int *S = new unsigned int[len + addLen];
	if (fread(S, (size_t)4, (size_t)len, InFile) != (size_t)len) {
		cout << "Error reading file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return S;
}

unsigned long long *readFileLong(char *inFileName, unsigned int &len, unsigned int addLen) {
	FILE *InFile = openFile(inFileName, 8, len);
	unsigned long long *S = new unsigned long long[len + addLen];
	if (fread(S, (size_t)8, (size_t)len, InFile) != (size_t)len) {
		cout << "Error reading file " << inFileName << endl;
		exit(1);
	}
	fclose(InFile);
	return S;
}

bool fileExists(char *inFileName) {
	FILE *InFile;
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

unsigned char *readText(char *inFileName, unsigned int &textLen, unsigned char eof) {
	unsigned char *S = readFileChar(inFileName, textLen, 1);
	S[textLen] = eof;
	return S;
}

void checkNullChar(unsigned char *text, unsigned int textLen) {
	for (unsigned int i = 0; i < textLen; ++i) {
		if (text[i] == '\0') {
			cout << "Error: text contains at least one 0 character" << endl;
			exit(1);
		}
	}
}

unsigned int *getSA(unsigned char *text, unsigned int textLen, unsigned int &saLen, unsigned int addLen, bool verbose) {
	saLen = textLen + 1;
	if (verbose) cout << "Creating SA ... " << flush;
	unsigned int *sa = new unsigned int[saLen + addLen];
	sa[0] = textLen;
	++sa;
	sais(text, (int *)sa, textLen);
	--sa;
	if (verbose) cout << "Done" << endl;
	return sa;
}

unsigned char *getBWT(unsigned char *text, unsigned int textLen, unsigned int &bwtLen, unsigned char eof, bool verbose) {
	unsigned int saLen;
	unsigned int *sa = getSA(text, textLen, saLen, 0, verbose);
	if (verbose) cout << "Creating BWT ... " << flush;
	bwtLen = textLen + 1;
	unsigned char *bwt = new unsigned char[bwtLen + 1];
	bwt[bwtLen] = eof;
	bwt[0] = text[textLen - 1];
	for (unsigned int i = 1; i < saLen; ++i) {
		if (sa[i] == 0) bwt[i] = eof;
		else bwt[i] = text[sa[i] - 1];
	}
	delete[] sa;
	if (verbose) cout << "Done" << endl;
	return bwt;
}

void fillArrayC(unsigned char *text, unsigned int textLen, unsigned int* C, bool verbose) {
	if (verbose) cout << "Creating array C ... " << flush;
	for (int i = 0; i < 257; ++i) C[i] = 0;
	for (unsigned int i = 0; i < textLen; ++i) {
		++C[text[i] + 1];
	}
	C[0] = 1;
	for (int i = 0; i < 256; ++i) {
		C[i + 1] += C[i];
	}
	if (verbose) cout << "Done" << endl;
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
