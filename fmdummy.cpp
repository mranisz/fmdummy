#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include "fmdummy.h"
#include "shared/common.h"

using namespace std;

/*FMDUMMY1*/

void FMDummy1::setType(string indexType) {
	if (indexType == "512c") this->type = FMDummy1::TYPE_512c;
	else this->type = FMDummy1::TYPE_256c;
	this->setFunctions();
}

void FMDummy1::setSelectedChars(string selectedChars) {
	if (selectedChars == "all") this->allChars = true;
	else {
		this->allChars = false;
		this->ordChars = breakByDelimeter(selectedChars, '.', this->ordCharsLen);
	}
}

void FMDummy1::setFunctions() {
	switch (this->type) {
	case FMDummy1::TYPE_512c:
		this->builder = &buildRank_512_counter40;
		this->countOperation = &count_512_counter40;
		break;
	default:
		this->builder = &buildRank_256_counter48;
		this->countOperation = &count_256_counter48;
	}
}

void FMDummy1::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy1::initialize() {
	this->raw_bwtWithRanks = NULL;
	this->bwtWithRanks = NULL;

	this->raw_bwtWithRanksLen = 0;

	this->c = NULL;
	this->type = FMDummy1::TYPE_256c;
	this->ordCharsLen = 0;
	this->ordChars = NULL;
	this->allChars = true;
	this->textSize = 0;

	this->builder = NULL;
	this->countOperation = NULL;
}

void FMDummy1::freeMemory() {
	if (this->raw_bwtWithRanks != NULL) for (unsigned int i = 0; i < 256; ++i) if (this->raw_bwtWithRanks[i] != NULL) delete[] this->raw_bwtWithRanks[i];
	if (this->bwtWithRanks != NULL) delete[] this->bwtWithRanks;
	if (this->ordChars != NULL) delete[] this->ordChars;
}

void FMDummy1::build(unsigned char* text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->textSize = textLen;
	if (this->allChars) {
		if (this->verbose) cout << "Counting char frequencies ... " << flush;
		unsigned int charsFreq[256];
		for (unsigned int i = 0; i < 256; ++i) charsFreq[i] = 0;
		this->ordCharsLen = 0;
		for (unsigned int i = 0; i < textLen; ++i) {
			if (charsFreq[(unsigned int)text[i]] == 0) ++this->ordCharsLen;
			++charsFreq[(unsigned int)text[i]];
		}
		if (this->verbose) cout << "Done" << endl;
		if (this->ordCharsLen > 16) {
			cout << "Error building index: text cannot contain more than 16 unique symbols" << endl;
			exit(1);
		}
		this->ordChars = new unsigned int[this->ordCharsLen];
		unsigned int counter = 0;
		for (unsigned int i = 0; i < 256; ++i) if (charsFreq[i] > 0) this->ordChars[counter++] = i;
	}
	unsigned int bwtLen;
	unsigned char *bwt = getBWT(text, textLen, bwtLen, 0, this->verbose);
	if (this->verbose) cout << "Compacting BWT for selected chars ... " << flush;
	unsigned int bwtDenseLen = (bwtLen / 8);
	if (bwtLen % 8 > 0) ++bwtDenseLen;
	unsigned int bwtDenseInLongLen = bwtDenseLen / sizeof(unsigned long long);
	if (bwtDenseLen % sizeof(unsigned long long) > 0) ++bwtDenseInLongLen;
	unsigned long long *bwtDenseInLong[256];
	for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
		int ordChar = this->ordChars[i];
		unsigned char *bwtDense = getBinDenseForChar(bwt, bwtLen, ordChar);
		bwtDenseInLong[ordChar] = new unsigned long long[bwtDenseInLongLen + 8];
		for (unsigned long long j = 0; j < bwtDenseInLongLen; ++j) {
			bwtDenseInLong[ordChar][j] = ((unsigned long long)bwtDense[8 * j + 7] << 56) | ((unsigned long long)bwtDense[8 * j + 6] << 48) | ((unsigned long long)bwtDense[8 * j + 5] << 40) | ((unsigned long long)bwtDense[8 * j + 4] << 32) | ((unsigned long long)bwtDense[8 * j + 3] << 24) | ((unsigned long long)bwtDense[8 * j + 2] << 16) | ((unsigned long long)bwtDense[8 * j + 1] << 8) | (unsigned long long)bwtDense[8 * j];
		}
		for (unsigned long long j = bwtDenseInLongLen; j < bwtDenseInLongLen + 8; ++j) {
			bwtDenseInLong[ordChar][j] = 0ULL;
		}
		delete[] bwtDense;
	}
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;

	this->c = getArrayC(text, textLen, verbose);

	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	this->raw_bwtWithRanks = builder(bwtDenseInLong, bwtDenseInLongLen, this->ordChars, this->ordCharsLen, this->raw_bwtWithRanksLen);
	this->bwtWithRanks = new unsigned long long*[256];
	for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
		unsigned int c = this->ordChars[i];
		this->bwtWithRanks[c] = this->raw_bwtWithRanks[c];
		while ((unsigned long long)(this->bwtWithRanks[c]) % 128) ++(this->bwtWithRanks[c]);
	}
	if (this->verbose) cout << "Done" << endl;

	for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
		delete[] bwtDenseInLong[this->ordChars[i]];
	}
	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummy1::getIndexSize() {
	return (sizeof(this->type) + sizeof(this->ordCharsLen) + this->ordCharsLen * sizeof(unsigned char) + sizeof(this->allChars) + 257 * sizeof(unsigned int) + 256 * sizeof(unsigned long long*) + this->ordCharsLen * this->raw_bwtWithRanksLen * sizeof(unsigned long long));
}

unsigned int FMDummy1::getTextSize() {
	return this->textSize;
}

unsigned int FMDummy1::count(unsigned char *pattern, unsigned int patternLen) {
	return this->countOperation(pattern, patternLen - 1, this->c, this->bwtWithRanks, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1]);
}

unsigned int *FMDummy1::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummy1::save(char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	FILE* outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->allChars, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->ordCharsLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->ordChars, (size_t)sizeof(unsigned int), (size_t)this->ordCharsLen, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(&this->raw_bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
		fwrite(this->raw_bwtWithRanks[this->ordChars[i]], (size_t)sizeof(unsigned long long), (size_t)this->raw_bwtWithRanksLen, outFile);
	}
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummy1::load(char *fileName) {
	this->free();
	FILE* inFile;
	inFile = fopen(fileName, "rb");
	fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	this->setFunctions();
	fread(&this->allChars, (size_t)sizeof(bool), (size_t)1, inFile);
	fread(&this->ordCharsLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->ordChars = new unsigned int[this->ordCharsLen];
	fread(this->ordChars, (size_t)sizeof(unsigned int), (size_t)this->ordCharsLen, inFile);
	fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->c = new unsigned int[257];
	fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	fread(&this->raw_bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->raw_bwtWithRanks = new unsigned long long*[256];
	this->bwtWithRanks = new unsigned long long*[256];
	for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
		unsigned int c = this->ordChars[i];
		this->raw_bwtWithRanks[c] = new unsigned long long[this->raw_bwtWithRanksLen];
		fread(this->raw_bwtWithRanks[c], (size_t)sizeof(unsigned long long), (size_t)this->raw_bwtWithRanksLen, inFile);
		this->bwtWithRanks[c] = this->raw_bwtWithRanks[c];
		while ((unsigned long long)(this->bwtWithRanks[c]) % 128) ++(this->bwtWithRanks[c]);
	}
	fclose(inFile);
	if (this->verbose) cout << "Done" << endl;
}

/*FMDUMMY2*/

void FMDummy2::setType(string indexType, string schema) {
	if (indexType == "512c") this->type = FMDummy2::TYPE_512c;
	else this->type = FMDummy2::TYPE_256c;
	if (schema == "CB") this->schema = FMDummy2::SCHEMA_CB;
	else this->schema = FMDummy2::SCHEMA_SCBO;
	this->setFunctions();
}

void FMDummy2::setBitsPerChar(string bitsPerChar) {
	if (bitsPerChar == "3") this->bitsPerChar = FMDummy2::BITS_3;
	else this->bitsPerChar = FMDummy2::BITS_4;
}

void FMDummy2::setFunctions() {
	switch (this->type) {
	case FMDummy2::TYPE_512c:
		this->builder = &buildRank_512_counter40;
		switch (this->schema) {
		case FMDummy2::SCHEMA_CB:
			this->countOperation = &count_CB_512_counter40;
			break;
		default:
			this->countOperation = &count_SCBO_512_counter40;
		}
		break;
	default:
		this->builder = &buildRank_256_counter48;
		switch (this->schema) {
		case FMDummy2::SCHEMA_CB:
			this->countOperation = &count_CB_256_counter48;
			break;
		default:
			this->countOperation = &count_SCBO_256_counter48;
		}
	}
}

void FMDummy2::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy2::initialize() {
	this->raw_bwtWithRanks = NULL;
	this->bwtWithRanks = NULL;
	this->raw_bwtWithRanksLen = 0;

	for (int i = 0; i < 256; ++i) {
		this->encodedChars[i] = NULL;
	}
	for (int i = 0; i < 256; ++i) {
		this->encodedCharsLen[i] = 0;
	}

	this->c = NULL;
	this->type = FMDummy2::TYPE_256c;
	this->schema = FMDummy2::SCHEMA_SCBO;
	this->bitsPerChar = FMDummy2::BITS_4;
	this->bInC = 0;
	this->textSize = 0;

	this->builder = NULL;
	this->countOperation = NULL;
}

void FMDummy2::freeMemory() {
	if (this->raw_bwtWithRanks != NULL) for (unsigned int i = 0; i < 256; ++i) if (this->raw_bwtWithRanks[i] != NULL) delete[] this->raw_bwtWithRanks[i];
	for (unsigned int i = 0; i < 256; ++i) if (this->encodedChars[i] != NULL) delete[] this->encodedChars[i];
	if (this->bwtWithRanks != NULL) delete[] this->bwtWithRanks;
}

void FMDummy2::build(unsigned char *text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->textSize = textLen;
	unsigned int encodedTextLen;
	unsigned char *encodedText = NULL;
	unsigned int b = 0;
	switch (this->schema) {
	case FMDummy2::SCHEMA_SCBO:
		encodedText = getEncodedInSCBO(this->bitsPerChar, text, textLen, encodedTextLen, this->encodedChars, this->encodedCharsLen);
		break;
	case FMDummy2::SCHEMA_CB:
		encodedText = getEncodedInCB(this->bitsPerChar, text, textLen, encodedTextLen, this->encodedChars, this->encodedCharsLen, b);
		break;
	}
	unsigned int bwtLen;
	unsigned int ordCharsLen = (unsigned int)pow(2.0, (double)this->bitsPerChar);
	unsigned char *bwt = getBWT(encodedText, encodedTextLen, bwtLen, ordCharsLen, this->verbose);
	if (this->verbose) cout << "Compacting BWT ... " << flush;
	unsigned int bwtDenseLen = (bwtLen / 8);
	if (bwtLen % 8 > 0) ++bwtDenseLen;
	unsigned int bwtDenseInLongLen = bwtDenseLen / sizeof(unsigned long long);
	if (bwtDenseLen % sizeof(unsigned long long) > 0) ++bwtDenseInLongLen;
	unsigned long long *bwtDenseInLong[256];
	unsigned int *ordChars = new unsigned int[ordCharsLen];
	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		ordChars[i] = i;
		unsigned char *bwtDense = getBinDenseForChar(bwt, bwtLen, i);
		bwtDenseInLong[i] = new unsigned long long[bwtDenseInLongLen + 8];
		for (unsigned long long j = 0; j < bwtDenseInLongLen; ++j) {
			bwtDenseInLong[i][j] = ((unsigned long long)bwtDense[8 * j + 7] << 56) | ((unsigned long long)bwtDense[8 * j + 6] << 48) | ((unsigned long long)bwtDense[8 * j + 5] << 40) | ((unsigned long long)bwtDense[8 * j + 4] << 32) | ((unsigned long long)bwtDense[8 * j + 3] << 24) | ((unsigned long long)bwtDense[8 * j + 2] << 16) | ((unsigned long long)bwtDense[8 * j + 1] << 8) | (unsigned long long)bwtDense[8 * j];
		}
		for (unsigned long long j = bwtDenseInLongLen; j < bwtDenseInLongLen + 8; ++j) {
			bwtDenseInLong[i][j] = 0ULL;
		}
		delete[] bwtDense;
	}
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;

	this->c = getArrayC(encodedText, encodedTextLen, verbose);
	if (this->schema == FMDummy2::SCHEMA_CB) this->bInC = this->c[b];

	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	this->raw_bwtWithRanks = builder(bwtDenseInLong, bwtDenseInLongLen, ordChars, ordCharsLen, this->raw_bwtWithRanksLen);
	this->bwtWithRanks = new unsigned long long*[256];
	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		this->bwtWithRanks[i] = this->raw_bwtWithRanks[i];
		while ((unsigned long long)(this->bwtWithRanks[i]) % 128) ++(this->bwtWithRanks[i]);
	}
	if (this->verbose) cout << "Done" << endl;

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		delete[] bwtDenseInLong[i];
	}
	delete[] ordChars;
	delete[] encodedText;
	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummy2::getIndexSize() {
	unsigned int encodedCharsLenSum = 0;
	for (int i = 0; i < 256; ++i) {
		encodedCharsLenSum += this->encodedCharsLen[i];
	}
	unsigned int size = (sizeof(this->type) + 257 * sizeof(unsigned int) + 256 * sizeof(unsigned long long*) + (unsigned int)pow(2.0, (double)this->bitsPerChar) * this->raw_bwtWithRanksLen * sizeof(unsigned long long) + 256 * sizeof(unsigned int) + encodedCharsLenSum * sizeof(unsigned char));
	if (this->schema == FMDummy2::SCHEMA_CB) size += sizeof(unsigned int);
	return size;
}

unsigned int FMDummy2::getTextSize() {
	return this->textSize;
}

unsigned int FMDummy2::count(unsigned char *pattern, unsigned int patternLen) {
	bool wrongEncoding = false;
	unsigned int encodedPatternLen;
	unsigned char *encodedPattern = encodePattern(pattern, patternLen, this->encodedChars, this->encodedCharsLen, encodedPatternLen, wrongEncoding);
	unsigned int count;
	if (wrongEncoding) count = 0;
	else count = this->countOperation(encodedPattern, encodedPatternLen, this->c, this->bwtWithRanks, this->bInC);
	delete[] encodedPattern;
	return count;
}

unsigned int *FMDummy2::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummy2::save(char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	FILE* outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->schema, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->bitsPerChar, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(this->encodedCharsLen, (size_t)sizeof(unsigned int), (size_t)256, outFile);
	for (int i = 0; i < 256; ++i) {
		if (this->encodedChars[i] != NULL) fwrite(this->encodedChars[i], (size_t)sizeof(unsigned char), (size_t)this->encodedCharsLen[i], outFile);
	}
	unsigned int maxChar = (unsigned int)pow(2.0, (double)this->bitsPerChar);
	fwrite(&this->raw_bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	for (unsigned int i = 0; i < maxChar; ++i) {
		fwrite(this->raw_bwtWithRanks[i], (size_t)sizeof(unsigned long long), (size_t)this->raw_bwtWithRanksLen, outFile);
	}
	if (this->schema == FMDummy2::SCHEMA_CB) fwrite(&this->bInC, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummy2::load(char *fileName) {
	this->free();
	FILE* inFile;
	inFile = fopen(fileName, "rb");
	fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	fread(&this->schema, (size_t)sizeof(int), (size_t)1, inFile);
	this->setFunctions();
	fread(&this->bitsPerChar, (size_t)sizeof(int), (size_t)1, inFile);
	fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->c = new unsigned int[257];
	fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	fread(this->encodedCharsLen, (size_t)sizeof(unsigned int), (size_t)256, inFile);
	for (int i = 0; i < 256; ++i) {
		if (this->encodedCharsLen[i] == 0) continue;
		this->encodedChars[i] = new unsigned char[this->encodedCharsLen[i]];
		fread(this->encodedChars[i], (size_t)sizeof(unsigned char), (size_t)this->encodedCharsLen[i], inFile);
	}
	unsigned int maxChar = (unsigned int)pow(2.0, (double)this->bitsPerChar);
	fread(&this->raw_bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->raw_bwtWithRanks = new unsigned long long*[256];
	this->bwtWithRanks = new unsigned long long*[256];
	for (unsigned int i = 0; i < maxChar; ++i) {
		this->raw_bwtWithRanks[i] = new unsigned long long[this->raw_bwtWithRanksLen];
		fread(this->raw_bwtWithRanks[i], (size_t)sizeof(unsigned long long), (size_t)this->raw_bwtWithRanksLen, inFile);
		this->bwtWithRanks[i] = this->raw_bwtWithRanks[i];
		while ((unsigned long long)(this->bwtWithRanks[i]) % 128) ++(this->bwtWithRanks[i]);
	}
	if (this->schema == FMDummy2::SCHEMA_CB) fread(&this->bInC, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	fclose(inFile);
	if (this->verbose) cout << "Done" << endl;

}

/*FMDUMMY3*/

void FMDummy3::setType(string indexType) {
	if (indexType == "1024") this->type = FMDummy3::TYPE_1024;
	else this->type = FMDummy3::TYPE_512;
	this->setFunctions();
}

void FMDummy3::setFunctions() {
	switch (this->type) {
	case FMDummy3::TYPE_1024:
		this->builder = &buildRank_1024_enc125;
		this->countOperation = &count_1024_enc125;
		break;
	default:
		this->builder = &buildRank_512_enc125;
		this->countOperation = &count_512_enc125;
	}
}

void FMDummy3::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy3::initialize() {
	this->raw_bwtWithRanks = NULL;
	this->bwtWithRanks = NULL;
	this->raw_bwtWithRanksLen = 0;

	this->c = NULL;
	this->type = FMDummy3::TYPE_512;
	this->textSize = 0;

	this->builder = NULL;
	this->countOperation = NULL;
}

void FMDummy3::freeMemory() {
	if (this->raw_bwtWithRanks != NULL) delete[] this->raw_bwtWithRanks;
}

void FMDummy3::build(unsigned char *text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->textSize = textLen;
	if (this->verbose) cout << "Converting text ... " << flush;
	unsigned char *convertedText = new unsigned char[textLen];
	for (unsigned int i = 0; i < textLen; ++i) {
		switch (text[i]) {
		case 'A': case 'C': case 'G': case 'T':
			convertedText[i] = text[i];
			break;
		default:
			convertedText[i] = 'N';
		}
	}
	if (this->verbose) cout << "Done" << endl;
	unsigned int bwtLen;
	unsigned char *bwt = getBWT(convertedText, textLen, bwtLen, 0, this->verbose);
	if (this->verbose) cout << "Encoding BWT ... " << flush;
	unsigned int selectedOrdChars[4] = { (unsigned int)'A', (unsigned int)'C', (unsigned int)'G', (unsigned int)'T' };
	unsigned int bwtEnc125Len;
	unsigned char *bwtEnc125 = encode125(bwt, bwtLen, selectedOrdChars, bwtEnc125Len);
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;
	fill125LUT(selectedOrdChars, this->lut);
	this->c = getArrayC(convertedText, textLen, verbose);
	delete[] convertedText;
	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	this->raw_bwtWithRanks = builder(bwtEnc125, bwtEnc125Len, this->lut, this->raw_bwtWithRanksLen);
	this->bwtWithRanks = raw_bwtWithRanks;
	while ((unsigned long long)this->bwtWithRanks % 128) ++this->bwtWithRanks;
	if (this->verbose) cout << "Done" << endl;
	delete[] bwtEnc125;
	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummy3::getIndexSize() {
	return (sizeof(this->type) + 257 * sizeof(unsigned int) + sizeof(unsigned char*) + this->raw_bwtWithRanksLen * sizeof(unsigned char) + 256 * 125 * sizeof(unsigned int));
}

unsigned int FMDummy3::getTextSize() {
	return this->textSize;
}

unsigned int FMDummy3::count(unsigned char *pattern, unsigned int patternLen) {
	return this->countOperation(pattern, patternLen - 1, this->c, this->bwtWithRanks, this->lut, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1]);
}

unsigned int *FMDummy3::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummy3::save(char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	FILE* outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(this->lut, (size_t)sizeof(unsigned int), (size_t)(256 * 125), outFile);
	fwrite(&this->raw_bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->raw_bwtWithRanks, (size_t)sizeof(unsigned char), (size_t)this->raw_bwtWithRanksLen, outFile);
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummy3::load(char *fileName) {
	this->free();
	FILE* inFile;
	inFile = fopen(fileName, "rb");
	fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	this->setFunctions();
	fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->c = new unsigned int[257];
	fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	fread(this->lut, (size_t)sizeof(unsigned int), (size_t)(256 * 125), inFile);
	fread(&this->raw_bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	this->raw_bwtWithRanks = new unsigned char[this->raw_bwtWithRanksLen];
	fread(this->raw_bwtWithRanks, (size_t)sizeof(unsigned char), (size_t)this->raw_bwtWithRanksLen, inFile);
	this->bwtWithRanks = raw_bwtWithRanks;
	while ((unsigned long long)this->bwtWithRanks % 128) ++this->bwtWithRanks;
	fclose(inFile);
	if (this->verbose) cout << "Done" << endl;

}

/*SHARED FUNCTIONS*/

unsigned char *getBinDenseForChar(unsigned char *bwt, unsigned int bwtLen, int ordChar) {
	unsigned int bwtDenseLen = bwtLen / 8;
	if (bwtLen % 8 > 0) ++bwtDenseLen;
	unsigned char *bwtDense = new unsigned char[bwtDenseLen];
	unsigned int curr = 0, temp = 0;

	for (unsigned int i = 0; i < bwtLen; ++i) {
		if (bwt[i] == ordChar) temp |= (1 << (i % 8));
		if (i % 8 == 7) {
			bwtDense[curr] = (unsigned char)temp;
			++curr;
			temp = 0;
		}
	}

	if (bwtLen % 8 > 0) bwtDense[curr] = (unsigned char)temp;
	return bwtDense;
}

unsigned long long** buildRank_256(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
	//rank should be 32bit
	unsigned long long *p;
	unsigned int rank;
	unsigned long long **result = new unsigned long long*[256];
	raw_bwtWithRanksLen = (bwtInLongLen + (bwtInLongLen * 64) / 192 + 1 + 16);

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		unsigned int c = ordChars[i];
		unsigned long long *resRank = new unsigned long long[(bwtInLongLen * 64) / 192 + 1];
		resRank[0] = 0;
		p = bwtInLong[c];
		rank = 0;
		for (long long i = 64; p < bwtInLong[c] + bwtInLongLen; ++p, i += 64) {
			rank += __builtin_popcountll(*p);
			if (i % 192 == 0) resRank[i / 192] = rank;
		}
		raw_bwtWithRanks[c] = new unsigned long long[raw_bwtWithRanksLen];
		unsigned long long *res = raw_bwtWithRanks[c];
		while ((unsigned long long)res % 128) {
			++res;
		}
		p = bwtInLong[c];
		long long counter = 0;
		for (long long i = 0; p < bwtInLong[c] + bwtInLongLen; ++p, ++i) {
			if (i % 3 == 0) res[counter++] = resRank[i / 3];
			res[counter++] = *p;
		}
		delete[] resRank;
		result[c] = res;
	}
	return result;
}

unsigned long long** buildRank_256_counter48(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
	unsigned long long *p, pops, rank, b1, b2;
	unsigned long long **raw_bwtWithRanks = new unsigned long long*[256];
	for (int i = 0; i < 256; ++i) raw_bwtWithRanks[i] = NULL;
	raw_bwtWithRanksLen = (bwtInLongLen + (bwtInLongLen * 64) / 192 + 1 + 16);

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		unsigned int c = ordChars[i];
		unsigned long long *resRank = new unsigned long long[(bwtInLongLen * 64) / 192 + 1];
		p = bwtInLong[c];
		rank = 0;
		pops = 0;
		for (unsigned long long i = 0; p < bwtInLong[c] + bwtInLongLen; p += 3, ++i) {
			pops = __builtin_popcountll(*p);
			b1 = (pops << 56);
			pops += __builtin_popcountll(*(p + 1));
			b2 = (pops << 48);
			pops += __builtin_popcountll(*(p + 2));
			resRank[i] = rank + b1 + b2;
			rank += pops;
			pops = 0;
		}
		raw_bwtWithRanks[c] = new unsigned long long[raw_bwtWithRanksLen];
		unsigned long long *bwtWithRanks = raw_bwtWithRanks[c];
		while ((unsigned long long)bwtWithRanks % 128) ++bwtWithRanks;
		p = bwtInLong[c];
		long long counter = 0;
		for (long long i = 0; p < bwtInLong[c] + bwtInLongLen; ++p, ++i) {
			if (i % 3 == 0) bwtWithRanks[counter++] = resRank[i / 3];
			bwtWithRanks[counter++] = *p;
		}
		delete[] resRank;
	}
	return raw_bwtWithRanks;
}

unsigned long long** buildRank_512(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
	//rank should be 32bit
	unsigned long long *p;
	unsigned int rank;
	unsigned long long **result = new unsigned long long*[256];
	raw_bwtWithRanksLen = (bwtInLongLen + (bwtInLongLen * 64) / 448 + 1 + 16);

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		unsigned int c = ordChars[i];
		unsigned long long *resRank = new unsigned long long[(bwtInLongLen * 64) / 448 + 1];
		resRank[0] = 0;
		p = bwtInLong[c];
		rank = 0;
		for (long long i = 64; p < bwtInLong[c] + bwtInLongLen; ++p, i += 64) {
			rank += __builtin_popcountll(*p);
			if (i % 448 == 0) resRank[i / 448] = rank;
		}
		raw_bwtWithRanks[c] = new unsigned long long[raw_bwtWithRanksLen];
		unsigned long long *res = raw_bwtWithRanks[c];
		while ((unsigned long long)res % 128) {
			++res;
		}
		p = bwtInLong[c];
		long long counter = 0;
		for (long long i = 0; p < bwtInLong[c] + bwtInLongLen; ++p, ++i) {
			if (i % 7 == 0) res[counter++] = resRank[i / 7];
			res[counter++] = *p;
		}
		delete[] resRank;
		result[c] = res;
	}
	return result;
}

unsigned long long** buildRank_512_counter40(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
	unsigned long long *p, pop1, pop2, pop3, rank, b1, b2, b3;
	unsigned long long **raw_bwtWithRanks = new unsigned long long*[256];
	for (int i = 0; i < 256; ++i) raw_bwtWithRanks[i] = NULL;
	raw_bwtWithRanksLen = (bwtInLongLen + (bwtInLongLen * 64) / 448 + 1 + 16);

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		unsigned int c = ordChars[i];
		unsigned long long *resRank = new unsigned long long[(bwtInLongLen * 64) / 448 + 1];
		p = bwtInLong[c];
		rank = 0;
		for (unsigned long long i = 0; p < bwtInLong[c] + bwtInLongLen; p += 7, ++i) {
			pop1 = __builtin_popcountll(*p) + __builtin_popcountll(*(p + 1));
			b1 = (pop1 << 56);
			pop2 = __builtin_popcountll(*(p + 2)) + __builtin_popcountll(*(p + 3));
			b2 = (pop2 << 48);
			pop3 = __builtin_popcountll(*(p + 4)) + __builtin_popcountll(*(p + 5));
			b3 = (pop3 << 40);
			resRank[i] = rank + b1 + b2 + b3;
			rank += pop1 + pop2 + pop3 + __builtin_popcountll(*(p + 6));
		}
		raw_bwtWithRanks[c] = new unsigned long long[raw_bwtWithRanksLen];
		unsigned long long *bwtWithRanks = raw_bwtWithRanks[c];
		while ((unsigned long long)bwtWithRanks % 128) ++bwtWithRanks;
		p = bwtInLong[c];
		long long counter = 0;
		for (long long i = 0; p < bwtInLong[c] + bwtInLongLen; ++p, ++i) {
			if (i % 7 == 0) bwtWithRanks[counter++] = resRank[i / 7];
			bwtWithRanks[counter++] = *p;
		}
		delete[] resRank;
	}
	return raw_bwtWithRanks;
}

unsigned int getRank_256(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
	// rank should be 32bit
	unsigned long long* p;
	unsigned int rank;
	unsigned int j = i / 192;
	p = bwtWithRanks[c] + 4 * j;
	rank = *p;
	++p;
	i -= (j * 192);
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	unsigned int temp3 = 0;
	switch (i / 64) {
	case 2:
		temp3 = __builtin_popcountll(*p); ++p;
	case 1:
		temp2 = __builtin_popcountll(*p); ++p;
	case 0:
		temp1 = __builtin_popcountll(*p & ((1ULL << (i % 64)) - 1));
	}
	rank += temp1 + temp2 + temp3;
	return rank;
}

unsigned int count_256(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks){
	int i = m - 1;
	unsigned char c = Q[i];
	unsigned int firstVal = C[c] + 1;
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 4 * ((firstVal - 1) / 192), 0, 3);
	unsigned int lastVal = C[c + 1];
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 4 * (lastVal / 192), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_256(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 4 * ((firstVal - 1) / 192), 0, 3);
		lastVal = C[c] + getRank_256(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 4 * (lastVal / 192), 0, 3);
		--i;
	}

	if (firstVal <= lastVal)
	{
		c = Q[i - 1];
		firstVal = C[c] + getRank_256(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_256(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int getRank_256_counter48(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
	unsigned int j = i / 192;
	unsigned long long* p = bwtWithRanks[c] + 4 * j;
	unsigned int rank = (*p) & 0x00000000FFFFFFFFULL;
	unsigned int b1 = ((*p) >> 56) & 0x00000000000000FFULL;  // popcount for 64-bit prefix
	unsigned int b2 = ((*p) >> 48) & 0x00000000000000FFULL;  // popcount for 128-bit prefix

	++p;
	i -= (j * 192);

	switch (i / 64) {
	case 2:
		rank += b2 + __builtin_popcountll(*(p + 2) & ((1ULL << (i % 64)) - 1));
		break;
	case 1:
		rank += b1 + __builtin_popcountll(*(p + 1) & ((1ULL << (i % 64)) - 1));
		break;
	case 0:
		rank += __builtin_popcountll(*p & ((1ULL << (i % 64)) - 1));
	}

	return rank;
}

unsigned int count_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal) {
	unsigned char c;
	__builtin_prefetch(bwtWithRanks[pattern[i - 1]] + 4 * ((firstVal - 1) / 192), 0, 3);
	__builtin_prefetch(bwtWithRanks[pattern[i - 1]] + 4 * (lastVal / 192), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_256_counter48(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 4 * ((firstVal - 1) / 192), 0, 3);
		lastVal = C[c] + getRank_256_counter48(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 4 * (lastVal / 192), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_256_counter48(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_256_counter48(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int count_SCBO_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC) {
	return count_256_counter48(pattern, i - 1, C, bwtWithRanks, C[pattern[i - 1]] + 1, C[pattern[i - 1] + 1]);
}

unsigned int count_CB_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC) {
	return count_256_counter48(pattern, i, C, bwtWithRanks, 1, bInC);
}

unsigned int getRank_512(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
	// rank should be 32bit
	unsigned long long* p;
	unsigned int rank, j = i / 448;
	p = bwtWithRanks[c] + 8 * j;
	rank = *p;
	++p;
	i -= (j * 448);
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	unsigned int temp3 = 0;
	unsigned int temp4 = 0;
	unsigned int temp5 = 0;
	unsigned int temp6 = 0;
	unsigned int temp7 = 0;
	switch (i / 64) {
	case 6:
		temp7 = __builtin_popcountll(*p); ++p;
	case 5:
		temp6 = __builtin_popcountll(*p); ++p;
	case 4:
		temp5 = __builtin_popcountll(*p); ++p;
	case 3:
		temp4 = __builtin_popcountll(*p); ++p;
	case 2:
		temp3 = __builtin_popcountll(*p); ++p;
	case 1:
		temp2 = __builtin_popcountll(*p); ++p;
	case 0:
		temp1 = __builtin_popcountll(*p & ((1ULL << (i % 64)) - 1));
	}
	rank += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7;
	return rank;
}

unsigned int count_512(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks){
	int i = m - 1;
	unsigned char c = Q[i];
	unsigned int firstVal = C[c] + 1;
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 8 * ((firstVal - 1) / 448), 0, 3);
	unsigned int lastVal = C[c + 1];
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 8 * (lastVal / 448), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_512(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 8 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRank_512(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 8 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_512(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_512(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int getRank_512_counter40(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {

	unsigned int j = i / 448;
	unsigned long long* p = bwtWithRanks[c] + 8 * j;
	unsigned int rank = (*p) & 0x00000000FFFFFFFFULL;
	unsigned int b1 = ((*p) >> 56) & 0x00000000000000FFULL;  // popcount for 128-bit prefix
	unsigned int b2 = b1 + (((*p) >> 48) & 0x00000000000000FFULL);  // popcount for 256-bit prefix
	unsigned int b3 = b2 + (((*p) >> 40) & 0x00000000000000FFULL);  // popcount for 384-bit prefix
	unsigned int temp1, temp2;

	++p;
	i -= (j * 448);

	switch (i / 64) {
	case 6:
		rank += b3 + __builtin_popcountll(*(p + 6) & ((1ULL << (i % 64)) - 1));
		break;
	case 5:
		temp1 = __builtin_popcountll(*(p + 4));
		temp2 = __builtin_popcountll(*(p + 5) & ((1ULL << (i % 64)) - 1));
		rank += b2 + temp1 + temp2;
		break;
	case 4:
		rank += b2 + __builtin_popcountll(*(p + 4) & ((1ULL << (i % 64)) - 1));
		break;
	case 3:
		temp1 = __builtin_popcountll(*(p + 2));
		temp2 = __builtin_popcountll(*(p + 3) & ((1ULL << (i % 64)) - 1));
		rank += b1 + temp1 + temp2;
		break;
	case 2:
		rank += b1 + __builtin_popcountll(*(p + 2) & ((1ULL << (i % 64)) - 1));
		break;
	case 1:
		temp1 = __builtin_popcountll(*p);
		temp2 = __builtin_popcountll(*(p + 1) & ((1ULL << (i % 64)) - 1));
		rank += temp1 + temp2;
		break;
	case 0:
		rank += __builtin_popcountll(*p & ((1ULL << (i % 64)) - 1));
	}

	return rank;
}

unsigned int count_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal){
	unsigned char c = pattern[i];
	__builtin_prefetch(bwtWithRanks[pattern[i - 1]] + 8 * ((firstVal - 1) / 448), 0, 3);
	__builtin_prefetch(bwtWithRanks[pattern[i - 1]] + 8 * (lastVal / 448), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_512_counter40(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 8 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRank_512_counter40(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 8 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_512_counter40(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_512_counter40(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int count_SCBO_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC) {
	return count_512_counter40(pattern, i - 1, C, bwtWithRanks, C[pattern[i - 1]] + 1, C[pattern[i - 1] + 1]);
}

unsigned int count_CB_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC) {
	return count_512_counter40(pattern, i, C, bwtWithRanks, 1, bInC);
}

bool sortCharsCount(unsigned int* i, unsigned int* j) {
	return (i[1] > j[1]);
}

unsigned char *getEncodedInSCBO(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen) {

	int max = (int)pow(2.0, (double)bits);

	unsigned int charsCount[256][2];
	for (int i = 0; i < 256; ++i) {
		charsCount[i][0] = i;
		charsCount[i][1] = 0;
	}
	for (unsigned int i = 0; i < textLen; ++i) {
		charsCount[text[i]][1]++;
	}

	unsigned int charsLen = 0;
	for (int i = 0; i < 256; ++i) {
		if (charsCount[i][1] != 0) ++charsLen;
	}

	vector<unsigned int*> charsCountVector(charsCount, charsCount + 256);
	sort(charsCountVector.begin(), charsCountVector.end(), sortCharsCount);

	unsigned char chars[256];
	int i = 0;
	for (vector<unsigned int*>::iterator it = charsCountVector.begin(); it != charsCountVector.end(); ++it, ++i) {
		chars[i] = **it;
	}

	unsigned int totalTotal = (unsigned int)(-1);
	unsigned int best[4] = {0, 0, 0, 0};
	bool notFound = true;

	for (int o = 1; o < max - 2; ++o) {
		for (int b = 1; b < max - 2; ++b) {
			for (int c = 1; c < max - 2; ++c) {
				for (int s = 1; s < max - 2; ++s) {
					if (o + b + c + s != max) continue;
					int sig = charsLen;
					unsigned int total = 0;
					unsigned int curr = 0;
					if (sig > 0) {
						for (int i = 0; i < o; ++i) {
							total += charsCountVector[curr][1] * 1;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= o;
					}
					if (sig > 0) {
						for (int i = 0; i < b * s; ++i) {
							total += charsCountVector[curr][1] * 2;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= b * s;
					}
					if (sig > 0) {
						for (int i = 0; i < b * c * s; ++i) {
							total += charsCountVector[curr][1] * 3;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= b * c * s;
					}
					if (sig > 0) {
						for (int i = 0; i < b * c * c * s; ++i) {
							total += charsCountVector[curr][1] * 4;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= b * c * c * s;
					}
					if (sig > 0) {
						for (int i = 0; i < b * c * c * c * s; ++i) {
							total += charsCountVector[curr][1] * 5;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= b * c * c * c * s;
					}
					if (sig > 0) continue;
					if (total < totalTotal) {
						notFound = false;
						totalTotal = total;
						best[0] = o;
						best[1] = b;
						best[2] = s;
						best[3] = c;
					}
				}
			}
		}
	}
	if (notFound) {
		cout << "Error building index: text contains more symbols than can be encoded in SCBO schema on " << bits << " bits" << endl;
		exit(1);
	}

	unsigned int o = best[0];
	unsigned int b = best[1];
	unsigned int s = best[2];
	unsigned int c = best[3];

	unsigned int bStart = o;
	unsigned int sStart = bStart + b;
	unsigned int cStart = sStart + s;

	unsigned int off1, off2, off3, off4, off5;

	for (unsigned int i = 0; i < charsLen; ++i) {
		if (i < o) {
			encodedChars[chars[i]] = new unsigned char[1];
			encodedChars[chars[i]][0] = i;
			encodedCharsLen[chars[i]] = 1;
		}
		else if (i < o + b * s) {
			int j = i - o;
			encodedChars[chars[i]] = new unsigned char[2];
			encodedChars[chars[i]][0] = bStart + j / s;
			encodedChars[chars[i]][1] = sStart + j % s;
			encodedCharsLen[chars[i]] = 2;
		}
		else if (i < o + b * s + b * c * s) {
			int j = i - o - b * s;
			off3 = j % s;
			off2 = (j / s) % c;
			off1 = j / (s * c);
			encodedChars[chars[i]] = new unsigned char[3];
			encodedChars[chars[i]][0] = bStart + off1;
			encodedChars[chars[i]][1] = cStart + off2;
			encodedChars[chars[i]][2] = sStart + off3;
			encodedCharsLen[chars[i]] = 3;
		}
		else if (i < o + b * s + b * c * s + b * c * c * s) {
			int j = i - o - b * s - b * c * s;
			off4 = j % s;
			off3 = (j / s) % c;
			off2 = (j / (s * c)) % c;
			off1 = j / (s * c * c);
			encodedChars[chars[i]] = new unsigned char[4];
			encodedChars[chars[i]][0] = bStart + off1;
			encodedChars[chars[i]][1] = cStart + off2;
			encodedChars[chars[i]][2] = cStart + off3;
			encodedChars[chars[i]][3] = sStart + off4;
			encodedCharsLen[chars[i]] = 4;
		}
		else {
			int j = i - o - b * s - b * c * s - b * c * c * s;
			off5 = j % s;
			off4 = (j / s) % c;
			off3 = (j / (s * c)) % c;
			off2 = (j / (s * c * c)) % c;
			off1 = j / (s * c * c * c);
			encodedChars[chars[i]] = new unsigned char[5];
			encodedChars[chars[i]][0] = bStart + off1;
			encodedChars[chars[i]][1] = cStart + off2;
			encodedChars[chars[i]][2] = cStart + off3;
			encodedChars[chars[i]][3] = cStart + off4;
			encodedChars[chars[i]][4] = sStart + off5;
			encodedCharsLen[chars[i]] = 5;
		}
	}

	unsigned char *encodedText = new unsigned char[totalTotal];
	encodedTextLen = 0;

	for (unsigned int i = 0; i < textLen; ++i) {
		unsigned char ch = text[i];
		for (unsigned int j = 0; j < encodedCharsLen[ch]; ++j) {
			encodedText[encodedTextLen++] = encodedChars[ch][j];
		}
	}

	return encodedText;
}

unsigned char *getEncodedInCB(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen, unsigned int &b) {

	int max = (int)pow(2.0, (double)bits);

	unsigned int charsCount[256][2];
	for (int i = 0; i < 256; ++i) {
		charsCount[i][0] = i;
		charsCount[i][1] = 0;
	}
	for (unsigned int i = 0; i < textLen; ++i) {
		charsCount[text[i]][1]++;
	}

	unsigned int charsLen = 0;
	for (int i = 0; i < 256; ++i) {
		if (charsCount[i][1] != 0) ++charsLen;
	}

	vector<unsigned int*> charsCountVector(charsCount, charsCount + 256);
	sort(charsCountVector.begin(), charsCountVector.end(), sortCharsCount);

	unsigned char chars[256];
	int i = 0;
	for (vector<unsigned int*>::iterator it = charsCountVector.begin(); it != charsCountVector.end(); ++it, ++i) {
		chars[i] = **it;
	}

	unsigned int totalTotal = (unsigned int)(-1);
	unsigned int best[2] = {0, 0};
	bool notFound = true;

	for (int b = 1; b < max; ++b) {
		for (int c = 1; c < max; ++c) {
			if (b + c != max) continue;
			int sig = charsLen;
			unsigned int total = 0;
			unsigned int curr = 0;
			if (sig > 0) {
				for (int i = 0; i < b; ++i) {
					total += charsCountVector[curr][1] * 1;
					++curr;
					if (curr >= charsLen) break;
				}
				sig -= b;
			}
			if (sig > 0) {
				for (int i = 0; i < b * c; ++i) {
					total += charsCountVector[curr][1] * 2;
					++curr;
					if (curr >= charsLen) break;
				}
				sig -= b * c;
			}
			if (sig > 0) {
				for (int i = 0; i < b * c * c; ++i) {
					total += charsCountVector[curr][1] * 3;
					++curr;
					if (curr >= charsLen) break;
				}
				sig -= b * c * c;
			}
			if (sig > 0) {
				for (int i = 0; i < b * c * c * c; ++i) {
					total += charsCountVector[curr][1] * 4;
					++curr;
					if (curr >= charsLen) break;
				}
				sig -= b * c * c * c;
			}
			if (sig > 0) {
				for (int i = 0; i < b * c * c * c * c; ++i) {
					total += charsCountVector[curr][1] * 5;
					++curr;
					if (curr >= charsLen) break;
				}
				sig -= b * c * c * c * c;
			}
			if (sig > 0) continue;
			if (total < totalTotal) {
				notFound = false;
				totalTotal = total;
				best[0] = b;
				best[1] = c;
			}
		}
	}
	if (notFound) {
		cout << "Error building index: text contains more symbols than can be encoded in CB schema on " << bits << " bits" << endl;
		exit(1);
	}
	b = best[0];
	unsigned int c = best[1];

	unsigned int bStart = 0;
	unsigned int cStart = b;

	unsigned int off1, off2, off3, off4, off5;

	for (unsigned int i = 0; i < charsLen; ++i) {
		if (i < b) {
			encodedChars[chars[i]] = new unsigned char[1];
			encodedChars[chars[i]][0] = i;
			encodedCharsLen[chars[i]] = 1;
		}
		else if (i < b + b * c) {
			int j = i - b;
			encodedChars[chars[i]] = new unsigned char[2];
			encodedChars[chars[i]][0] = bStart + j / c;
			encodedChars[chars[i]][1] = cStart + j % c;
			encodedCharsLen[chars[i]] = 2;
		}
		else if (i < b + b * c + b * c * c) {
			int j = i - b - b * c;
			off3 = j % c;
			off2 = (j / c) % c;
			off1 = j / (c * c);
			encodedChars[chars[i]] = new unsigned char[3];
			encodedChars[chars[i]][0] = bStart + off1;
			encodedChars[chars[i]][1] = cStart + off2;
			encodedChars[chars[i]][2] = cStart + off3;
			encodedCharsLen[chars[i]] = 3;
		}
		else if (i < b + b * c + b * c * c + b * c * c * c) {
			int j = i - b - b * c - b * c * c;
			off4 = j % c;
			off3 = (j / c) % c;
			off2 = (j / (c * c)) % c;
			off1 = j / (c * c * c);
			encodedChars[chars[i]] = new unsigned char[4];
			encodedChars[chars[i]][0] = bStart + off1;
			encodedChars[chars[i]][1] = cStart + off2;
			encodedChars[chars[i]][2] = cStart + off3;
			encodedChars[chars[i]][3] = cStart + off4;
			encodedCharsLen[chars[i]] = 4;
		}
		else {
			int j = i - b - b * c - b * c * c - b * c * c * c;
			off5 = j % c;
			off4 = (j / c) % c;
			off3 = (j / (c * c)) % c;
			off2 = (j / (c * c * c)) % c;
			off1 = j / (c * c * c * c);
			encodedChars[chars[i]] = new unsigned char[5];
			encodedChars[chars[i]][0] = bStart + off1;
			encodedChars[chars[i]][1] = cStart + off2;
			encodedChars[chars[i]][2] = cStart + off3;
			encodedChars[chars[i]][3] = cStart + off4;
			encodedChars[chars[i]][4] = cStart + off5;
			encodedCharsLen[chars[i]] = 5;
		}
	}

	unsigned char *encodedText = new unsigned char[totalTotal];
	encodedTextLen = 0;

	for (unsigned int i = 0; i < textLen; ++i) {
		unsigned char ch = text[i];
		for (unsigned int j = 0; j < encodedCharsLen[ch]; ++j) {
			encodedText[encodedTextLen++] = encodedChars[ch][j];
		}
	}

	return encodedText;
}

unsigned char* encodePattern(unsigned char* pattern, unsigned int patternLen, unsigned char** encodedChars, unsigned int* encodedCharsLen, unsigned int &encodedPatternLen, bool &wrongEncoding) {
	unsigned char* encodedPattern = new unsigned char[5 * patternLen + 1];
	unsigned char* p = pattern;
	encodedPatternLen = 0;
	for (; p < pattern + patternLen; ++p) {
		if (encodedCharsLen[*p] == 0) {
			wrongEncoding = true;
			break;
		}
		for (unsigned int i = 0; i < encodedCharsLen[*p]; ++i) {
			encodedPattern[encodedPatternLen++] = encodedChars[*p][i];
		}
	}
	return encodedPattern;
}

unsigned char *encode125(unsigned char* text, unsigned int textLen, unsigned int *selectedOrdChars, unsigned int &encodedTextLen) {
	encodedTextLen = textLen / 3;
	if (textLen % 3 > 0) ++encodedTextLen;
	unsigned char *textEnc125 = new unsigned char[encodedTextLen];

	for (unsigned int i = 0; i < encodedTextLen; ++i) {
		int temp = 0;
		for (int k = 0; k < 3; ++k) {
			bool encoded = false;
			if (3 * i + k < textLen) {
				for (int j = 0; j < 4; ++j) {
					if (text[3 * i + k] == selectedOrdChars[j]) {
						encoded = true;
						temp += j * (int)pow(5.0, (double)k);
						break;
					}
				}
			}
			if (!encoded) {
				temp += 4 * (int)pow(5.0, (double)k);
			}
		}
		textEnc125[i] = (unsigned char)temp;
	}
	return textEnc125;
}

unsigned int occ(unsigned int *a, unsigned int aLen, unsigned int elem) {
	unsigned int occ = 0;
	for (unsigned int i = 0; i < aLen; ++i) if (a[i] == elem) ++occ;
	return occ;
}

void fill125LUT(unsigned int *selectedOrdChars, unsigned int lut[][125]) {
	for (int i = 0; i < 125; ++i) {
		unsigned int first = i % 5;
		unsigned int second = (i / 5) % 5;
		unsigned int third = i / 25;

		unsigned int a[3] = { first, second, third };

		lut[selectedOrdChars[0]][i] = 0;
		lut[selectedOrdChars[1]][i] = 0;
		lut[selectedOrdChars[2]][i] = 0;
		lut[selectedOrdChars[3]][i] = 0;
		lut['N'][i] = 0;

		switch (first) {
		case 0: case 1: case 2: case 3:
			lut[selectedOrdChars[first]][i] = occ(a, 3, first);
			break;
		default:
			lut['N'][i] = occ(a, 3, 4);
		}

		switch (second) {
		case 0: case 1: case 2: case 3:
			lut[selectedOrdChars[second]][i] = occ(a, 3, second);
			break;
		default:
			lut['N'][i] = occ(a, 3, 4);
		}

		switch (third) {
		case 0: case 1: case 2: case 3:
			lut[selectedOrdChars[third]][i] = occ(a, 3, third);
			break;
		default:
			lut['N'][i] = occ(a, 3, 4);
		}
	}
}

unsigned char *buildRank_512_enc125(unsigned char *bwtEnc125, unsigned int bwtLen, unsigned int lut[][125], unsigned int &raw_bwtWithRanksLen) {
	unsigned int rank[4] = {0, 0, 0, 0};
	unsigned char *p, signs[4] = { 'A', 'C', 'G', 'T' };

	unsigned int **resRank = new unsigned int *[4];
	for (int i = 0; i < 4; ++i) {
		resRank[i] = new unsigned int[(bwtLen * 8) / 384 + 1];
		resRank[i][0] = 0;
	}
	for (int s = 0; s < 4; ++s) {
		p = bwtEnc125;
		for (unsigned int i = 8; p < bwtEnc125 + bwtLen; ++p, i += 8) {
			rank[s] += lut[signs[s]][*p];
			if (i % 384 == 0) resRank[s][i / 384] = rank[s];
		}
	}
	raw_bwtWithRanksLen = bwtLen + 4 * 4 * ((bwtLen * 8) / 384 + 1) + 128;
	unsigned char *raw_bwtWithRanks = new unsigned char[raw_bwtWithRanksLen];
	unsigned char *bwtWithRanks = raw_bwtWithRanks;
	while ((unsigned long long)bwtWithRanks % 128) ++bwtWithRanks;
	p = bwtEnc125;
	unsigned int counter = 0;
	for (unsigned int i = 0; p < bwtEnc125 + bwtLen; ++p, ++i) {
		if (i % 48 == 0) {
			for (int s = 0; s < 4; ++s) {
				bwtWithRanks[counter++] = (resRank[s][i / 48] & 0x000000FFU);
				bwtWithRanks[counter++] = ((resRank[s][i / 48] & 0x0000FF00U) >> 8);
				bwtWithRanks[counter++] = ((resRank[s][i / 48] & 0x00FF0000U) >> 16);
				bwtWithRanks[counter++] = ((resRank[s][i / 48] & 0xFF000000U) >> 24);
			}
		}
		bwtWithRanks[counter++] = *p;
	}
	delete[] resRank;
	return raw_bwtWithRanks;
}

unsigned char *buildRank_1024_enc125(unsigned char *bwtEnc125, unsigned int bwtLen, unsigned int lut[][125], unsigned int &raw_bwtWithRanksLen) {
	unsigned int rank[4] = {0, 0, 0, 0};
	unsigned char *p, signs[4] = { 'A', 'C', 'G', 'T' };

	unsigned int **resRank = new unsigned int *[4];
	for (int i = 0; i < 4; ++i) {
		resRank[i] = new unsigned int[(bwtLen * 8) / 896 + 1];
		resRank[i][0] = 0;
	}
	for (int s = 0; s < 4; ++s) {
		p = bwtEnc125;
		for (unsigned int i = 8; p < bwtEnc125 + bwtLen; ++p, i += 8) {
			rank[s] += lut[signs[s]][*p];
			if (i % 896 == 0) resRank[s][i / 896] = rank[s];
		}
	}
	raw_bwtWithRanksLen = bwtLen + 4 * 4 * ((bwtLen * 8) / 896 + 1) + 128;
	unsigned char *raw_bwtWithRanks = new unsigned char[raw_bwtWithRanksLen];
	unsigned char *bwtWithRanks = raw_bwtWithRanks;
	while ((unsigned long long)bwtWithRanks % 128) ++bwtWithRanks;
	p = bwtEnc125;
	unsigned int counter = 0;
	for (unsigned int i = 0; p < bwtEnc125 + bwtLen; ++p, ++i) {
		if (i % 112 == 0) {
			for (int s = 0; s < 4; ++s) {
				bwtWithRanks[counter++] = (resRank[s][i / 112] & 0x000000FFU);
				bwtWithRanks[counter++] = ((resRank[s][i / 112] & 0x0000FF00U) >> 8);
				bwtWithRanks[counter++] = ((resRank[s][i / 112] & 0x00FF0000U) >> 16);
				bwtWithRanks[counter++] = ((resRank[s][i / 112] & 0xFF000000U) >> 24);
			}
		}
		bwtWithRanks[counter++] = *p;
	}
	delete[] resRank;
	return raw_bwtWithRanks;
}

unsigned int getRank_512_enc125(unsigned char c, unsigned int i, unsigned char *bwtWithRanks, unsigned int lut[][125]) {
	unsigned int rank;
	unsigned char *p, last;
	unsigned int j = i / 144;
	p = bwtWithRanks + 64 * j;
	switch (c) {
	case 'T':
		p += 4;
	case 'G':
		p += 4;
	case 'C':
		p += 4;
	default:
		memcpy(&rank, p, (size_t)4);
	}
	p = bwtWithRanks + 64 * j + 16;
	i -= (j * 144);
	for (unsigned int k = 0; k < i / 3; ++k) {
		rank += lut[c][*p];
		++p;
	}
	switch (i % 3) {
	case 2:
		last = (*p) % 25 + 100;
		rank += lut[c][last];
		break;
	case 1:
		last = (*p) % 5 + 120;
		rank += lut[c][last];
		break;
	}
	return rank;
}

unsigned int count_512_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal) {
	unsigned char c;
	__builtin_prefetch(bwtWithRanks + 64 * ((firstVal - 1) / 144), 0, 3);
	__builtin_prefetch(bwtWithRanks + 64 * (lastVal / 144), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_512_enc125(c, firstVal - 1, bwtWithRanks, lut) + 1;
		__builtin_prefetch(bwtWithRanks + 64 * ((firstVal - 1) / 144), 0, 3);
		lastVal = C[c] + getRank_512_enc125(c, lastVal, bwtWithRanks, lut);
		__builtin_prefetch(bwtWithRanks + 64 * (lastVal / 144), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_512_enc125(c, firstVal - 1, bwtWithRanks, lut) + 1;
		lastVal = C[c] + getRank_512_enc125(c, lastVal, bwtWithRanks, lut);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int getRank_1024_enc125(unsigned char c, unsigned int i, unsigned char *bwtWithRanks, unsigned int lut[][125]) {
	unsigned int rank;
	unsigned char *p, last;
	unsigned int j = i / 336;
	p = bwtWithRanks + 128 * j;
	switch (c) {
	case 'T':
		p += 4;
	case 'G':
		p += 4;
	case 'C':
		p += 4;
	default:
		memcpy(&rank, p, (size_t)4);
	}
	p = bwtWithRanks + 128 * j + 16;
	i -= (j * 336);
	for (unsigned int k = 0; k < i / 3; ++k) {
		rank += lut[c][*p];
		++p;
	}
	switch (i % 3) {
	case 2:
		last = (*p) % 25 + 100;
		rank += lut[c][last];
		break;
	case 1:
		last = (*p) % 5 + 120;
		rank += lut[c][last];
		break;
	}
	return rank;
}

unsigned int count_1024_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal){
	unsigned char c;
	__builtin_prefetch(bwtWithRanks + 128 * ((firstVal - 1) / 336), 0, 3);
	__builtin_prefetch(bwtWithRanks + 128 * (lastVal / 336), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_1024_enc125(c, firstVal - 1, bwtWithRanks, lut) + 1;
		__builtin_prefetch(bwtWithRanks + 128 * ((firstVal - 1) / 336), 0, 3);
		lastVal = C[c] + getRank_1024_enc125(c, lastVal, bwtWithRanks, lut);
		__builtin_prefetch(bwtWithRanks + 128 * (lastVal / 336), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRank_1024_enc125(c, firstVal - 1, bwtWithRanks, lut) + 1;
		lastVal = C[c] + getRank_1024_enc125(c, lastVal, bwtWithRanks, lut);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}
