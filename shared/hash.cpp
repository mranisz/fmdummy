#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "xxhash.h"
#include "hash.h"

using namespace std;

void HT::setK(unsigned int k) {
	if (k == 0) {
		cout << "Error: not valid k value" << endl;
		exit(1);
	}
	this->k = k;
}

void HT::setLoadFactor(double loadFactor) {
	if (loadFactor <= 0.0 || loadFactor >= 100.0) {
		cout << "Error: not valid loadFactor value" << endl;
		exit(1);
	}
	this->loadFactor = loadFactor;
}

void HT::freeMemory() {
	if (this->boundariesHT != NULL) delete[] this->boundariesHT;
	if (this->entriesHT != NULL) delete[] this->entriesHT;
}

void HT::initialize() {
	this->k = 8;
	this->loadFactor = 0.9;
	this->bucketsNum = 0;
	this->emptyValueHT = 0;
	this->boundariesHT = NULL;
	this->alignedBoundariesHT = NULL;
	this->entriesHT = NULL;
	this->alignedEntriesHT = NULL;
}

void HT::free() {
	this->freeMemory();
	this->initialize();
}

unsigned int HT::getHTSize() {
	int size = sizeof(this->loadFactor) + sizeof(this->k) + sizeof(this->bucketsNum) + sizeof(this->emptyValueHT) + 256 * 257 * sizeof(unsigned int) + sizeof(this->alignedBoundariesHT) + sizeof(this->alignedEntriesHT);
	if (this->boundariesHT != NULL) size += (2 * this->bucketsNum + 32) * sizeof(unsigned int);
	if (this->entriesHT != NULL) size += (this->bucketsNum * this->k + 128) * sizeof(unsigned char);
	return size;
}

void HT::save(FILE *outFile) {
	bool nullPointer = false;
	bool notNullPointer = true;
	fwrite(&this->loadFactor, (size_t)sizeof(double), (size_t)1, outFile);
	fwrite(&this->k, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->bucketsNum, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->emptyValueHT, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 257), outFile);
	if (this->boundariesHT != NULL) {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		fwrite(this->alignedBoundariesHT, (size_t)sizeof(unsigned int), (size_t)(2 * this->bucketsNum), outFile);
	} else {
		fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	}
	if (this->entriesHT != NULL) {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		fwrite(this->alignedEntriesHT, (size_t)sizeof(unsigned char), (size_t)(this->bucketsNum * this->k), outFile);
	} else {
		fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	}
}

void HT::load(FILE *inFile) {
	this->free();
	bool isNotNullPointer;
	size_t result;
	result = fread(&this->loadFactor, (size_t)sizeof(double), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	result = fread(&this->k, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	result = fread(&this->bucketsNum, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	result = fread(&this->emptyValueHT, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	result = fread(this->lut2, (size_t)sizeof(unsigned int), (size_t)(256 * 257), inFile);
	if (result != (256 * 257)) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->boundariesHT = new unsigned int[2 * this->bucketsNum + 32];
		this->alignedBoundariesHT = this->boundariesHT;
		while ((unsigned long long)(this->alignedBoundariesHT) % 128) ++(this->alignedBoundariesHT);
		result = fread(this->alignedBoundariesHT, (size_t)sizeof(unsigned int), (size_t)(2 * this->bucketsNum), inFile);
		if (result != (2 * this->bucketsNum)) {
			cout << "Error loading index" << endl;
			exit(1);
		}
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->entriesHT = new unsigned char[this->bucketsNum * this->k + 128];
		this->alignedEntriesHT = this->entriesHT;
		while ((unsigned long long)(this->alignedEntriesHT) % 128) ++(this->alignedEntriesHT);
		result = fread(this->alignedEntriesHT, (size_t)sizeof(unsigned char), (size_t)(this->bucketsNum * this->k), inFile);
		if (result != (this->bucketsNum * this->k)) {
			cout << "Error loading index" << endl;
			exit(1);
		}
	}
}

unsigned int HT::getUniqueSuffixNum(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen, unsigned int *ordChars, unsigned int ordCharsLen) {
	unsigned int uniqueSuffixNum = 0;
	bool selectedChars = (ordCharsLen != 0);

	unsigned char *lastPattern = new unsigned char[this->k + 1];
	for (unsigned int i = 0; i < this->k; ++i) lastPattern[i] = 255;
	lastPattern[this->k] = '\0';

	unsigned char *pattern = new unsigned char[this->k + 1];

	for (unsigned int i = 0; i < saLen; i++) {
		if (sa[i] > (textLen - this->k)) continue;
		strncpy((char *)pattern, (const char*)(text + sa[i]), this->k);
		pattern[this->k] = '\0';
		if (strcmp((char *)pattern, (const char*)lastPattern) == 0) continue;
		else {
			strcpy((char *)lastPattern, (const char*)pattern);
			if (selectedChars) {
				bool rejectPattern = false;
				for (unsigned int j = 0; j < this->k; ++j) {
					bool symbolInOrdChars = false;
					for (unsigned int l = 0; l < ordCharsLen; ++l) {
						if ((unsigned int)pattern[j] == ordChars[l]) {
							symbolInOrdChars = true;
							break;
						}
					}
					if (!symbolInOrdChars) {
						rejectPattern = true;
						break;
					}
				}
				if (rejectPattern) continue;
			}
			++uniqueSuffixNum;
		}
	}

	delete[] lastPattern;
	delete[] pattern;

	return uniqueSuffixNum;
}

unsigned int HT::getHashValue(unsigned char* str) {
	return XXH32(str, strlen((const char *)str), 0);
}

void HT::fillHTData(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen) {
	this->emptyValueHT = saLen;
	unsigned int hash = this->bucketsNum;
	this->boundariesHT = new unsigned int[2 * this->bucketsNum + 32];
	this->alignedBoundariesHT = this->boundariesHT;
	while ((unsigned long long)(this->alignedBoundariesHT) % 128) ++(this->alignedBoundariesHT);

	for (unsigned int i = 0; i < 2 * this->bucketsNum; ++i) this->alignedBoundariesHT[i] = this->emptyValueHT;

	unsigned char *lastPattern = new unsigned char[this->k + 1];
	for (unsigned int i = 0; i < this->k; ++i) lastPattern[i] = 255;
	lastPattern[this->k] = '\0';

	unsigned char *pattern = new unsigned char[this->k + 1];

	for (unsigned int i = 0; i < saLen; i++) {
		if (sa[i] > (textLen - this->k)) continue;
		strncpy((char *)pattern, (const char *)(text + sa[i]), this->k);
		pattern[this->k] = '\0';
		if (strcmp((char *)pattern, (const char *)lastPattern) == 0) continue;
		else {
			strcpy((char *)lastPattern, (const char *)pattern);
			if (hash != this->bucketsNum) this->alignedBoundariesHT[2 * hash + 1] = i;
			hash = getHashValue(pattern) % this->bucketsNum;
		}
		while (true) {
			if (this->alignedBoundariesHT[2 * hash] == this->emptyValueHT) {
				this->alignedBoundariesHT[2 * hash] = i;
				break;
			}
			else {
				hash = (hash + 1) % this->bucketsNum;
			}
		}
	}

	delete[] lastPattern;
	delete[] pattern;
}

void HT::build(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen) {
	unsigned int uniqueSuffixNum = this->getUniqueSuffixNum(text, textLen, sa, saLen);
	this->bucketsNum = (double)uniqueSuffixNum * (1.0 / this->loadFactor);
	this->fillHTData(text, textLen, sa, saLen);
	fillLUT2(this->lut2, text, sa, saLen);
}

void HT::fillHTDataWithEntries(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen, unsigned int *ordChars, unsigned int ordCharsLen) {
	bool selectedChars = (ordCharsLen != 0);
	this->emptyValueHT = saLen;
	unsigned int hash = this->bucketsNum;
	this->boundariesHT = new unsigned int[2 * this->bucketsNum + 32];
	this->alignedBoundariesHT = this->boundariesHT;
	while ((unsigned long long)(this->alignedBoundariesHT) % 128) ++(this->alignedBoundariesHT);
	this->entriesHT = new unsigned char[this->bucketsNum * this->k + 128];
	this->alignedEntriesHT = this->entriesHT;
	while ((unsigned long long)(this->alignedEntriesHT) % 128) ++(this->alignedEntriesHT);

	for (unsigned int i = 0; i < 2 * this->bucketsNum; ++i) this->alignedBoundariesHT[i] = this->emptyValueHT;

	unsigned char *lastPattern = new unsigned char[this->k + 1];
	for (unsigned int i = 0; i < this->k; ++i) lastPattern[i] = 255;
	lastPattern[this->k] = '\0';

	unsigned char *pattern = new unsigned char[this->k + 1];

	for (unsigned int i = 0; i < saLen; i++) {
		if (sa[i] > (textLen - this->k)) continue;
		strncpy((char *)pattern, (const char *)(text + sa[i]), this->k);
		pattern[this->k] = '\0';
		if (strcmp((char *)pattern, (const char *)lastPattern) == 0) continue;
		else {
			strcpy((char *)lastPattern, (const char *)pattern);
			if (hash != this->bucketsNum) this->alignedBoundariesHT[2 * hash + 1] = i;
			if (selectedChars) {
				bool rejectPattern = false;
				for (unsigned int j = 0; j < this->k; ++j) {
					bool symbolInOrdChars = false;
					for (unsigned int l = 0; l < ordCharsLen; ++l) {
						if ((unsigned int)pattern[j] == ordChars[l]) {
							symbolInOrdChars = true;
							break;
						}
					}
					if (!symbolInOrdChars) {
						rejectPattern = true;
						break;
					}
				}
				if (rejectPattern) {
					hash = this->bucketsNum;
					continue;
				}
			}
			hash = getHashValue(pattern) % this->bucketsNum;
		}
		while (true) {
			if (this->alignedBoundariesHT[2 * hash] == this->emptyValueHT) {
				this->alignedBoundariesHT[2 * hash] = i;
				for (unsigned int j = 0; j < this->k; ++j) this->alignedEntriesHT[hash * this->k + j] = pattern[j];
				break;
			}
			else {
				hash = (hash + 1) % this->bucketsNum;
			}
		}
	}

	delete[] lastPattern;
	delete[] pattern;
}

void HT::buildWithEntries(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen, unsigned int *ordChars, unsigned int ordCharsLen) {
	unsigned int uniqueSuffixNum = this->getUniqueSuffixNum(text, textLen, sa, saLen, ordChars, ordCharsLen);
	this->bucketsNum = (double)uniqueSuffixNum * (1.0 / this->loadFactor);
	this->fillHTDataWithEntries(text, textLen, sa, saLen, ordChars, ordCharsLen);
	fillLUT2(this->lut2, text, sa, saLen);
}

void HT::getBoundaries(unsigned char *pattern, unsigned char *text, unsigned int *sa, unsigned int &leftBoundary, unsigned int &rightBoundary) {
	unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]];
	unsigned int rightBoundaryLUT2 = *(&(this->lut2[pattern[0]][pattern[1]]) + 1);
	if (leftBoundaryLUT2 < rightBoundaryLUT2) {
		unsigned char kChar = pattern[this->k];
		pattern[this->k] = '\0';
		unsigned int hash = this->getHashValue(pattern) % this->bucketsNum;
		while (true) {
			leftBoundary = this->alignedBoundariesHT[2 * hash];
			if (leftBoundary >= leftBoundaryLUT2 && leftBoundary < rightBoundaryLUT2 && strncmp((const char *)pattern, (const char *)(text + sa[leftBoundary]), this->k) == 0) {
				rightBoundary = this->alignedBoundariesHT[2 * hash + 1];
				break;
			}
			if (leftBoundary == this->emptyValueHT) {
				leftBoundary = 0;
				rightBoundary = 0;
				return;
			}
			++hash;
			if (hash == this->bucketsNum) {
				hash = 0;
			}
		}
		pattern[k] = kChar;
	} else {
		leftBoundary = 0;
		rightBoundary = 0;
	}
}

void HT::getBoundariesWithEntries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary) {
	unsigned int leftBoundaryLUT2 = this->lut2[pattern[0]][pattern[1]];
	unsigned int rightBoundaryLUT2 = *(&(this->lut2[pattern[0]][pattern[1]]) + 1);
	if (leftBoundaryLUT2 < rightBoundaryLUT2) {
		unsigned char kChar = pattern[this->k];
		pattern[this->k] = '\0';
		unsigned int hash = this->getHashValue(pattern) % this->bucketsNum;
		while (true) {
			leftBoundary = this->alignedBoundariesHT[2 * hash];
			if (leftBoundary >= leftBoundaryLUT2 && leftBoundary < rightBoundaryLUT2 && strncmp((const char *)pattern, (const char *)&(this->alignedEntriesHT[hash * this->k]), this->k) == 0) {
				rightBoundary = this->alignedBoundariesHT[2 * hash + 1];
				break;
			}
			if (leftBoundary == this->emptyValueHT) {
				leftBoundary = 0;
				rightBoundary = 0;
				return;
			}
			++hash;
			if (hash == this->bucketsNum) {
				hash = 0;
			}
		}
		pattern[k] = kChar;
	} else {
		leftBoundary = 0;
		rightBoundary = 0;
	}
}
