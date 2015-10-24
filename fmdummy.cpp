#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <queue>
#include <algorithm>
#include "shared/common.h"
#include "fmdummy.h"

using namespace std;

/*FMDUMMY1*/

void FMDummy1::setType(string indexType) {
	if (indexType == "512") this->type = FMDummy1::TYPE_512;
	else if (indexType == "256") this->type = FMDummy1::TYPE_256;
	else {
		cout << "Error: not valid index type" << endl;
		exit(1);
	}
}

void FMDummy1::setSelectedChars(string selectedChars) {
	this->selectedChars = selectedChars;
	if (selectedChars == "all") this->allChars = true;
	else this->allChars = false;
}

void FMDummy1::setK(unsigned int k) {
	if (k < 2) {
		cout << "Error: not valid k value" << endl;
		exit(1);
	}
	this->k = k;
}

void FMDummy1::setLoadFactor(double loadFactor) {
	if (loadFactor <= 0.0 || loadFactor >= 1.0) {
		cout << "Error: not valid loadFactor value" << endl;
		exit(1);
	}
	this->loadFactor = loadFactor;
}

void FMDummy1::setFunctions() {
	if (this->k > 1) {
		switch (this->type) {
		case FMDummy1::TYPE_512:
			this->builder = &buildRank_512_counter40;
			this->countOperation = &FMDummy1::count_hash_512_counter40;
			break;
		case FMDummy1::TYPE_256:
			this->builder = &buildRank_256_counter48;
			this->countOperation = &FMDummy1::count_hash_256_counter48;
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	} else {
		switch (this->type) {
		case FMDummy1::TYPE_512:
			this->builder = &buildRank_512_counter40;
			this->countOperation = &FMDummy1::count_std_512_counter40;
			break;
		case FMDummy1::TYPE_256:
			this->builder = &buildRank_256_counter48;
			this->countOperation = &FMDummy1::count_std_256_counter48;
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	}
}

void FMDummy1::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy1::initialize() {
	for (int i = 0; i < 256; ++i) this->bwtWithRanks[i] = NULL;
	this->alignedBWTWithRanks = NULL;
	this->bwtWithRanksLen = 0;
	this->ordCharsLen = 0;
	this->ordChars = NULL;
	for (int i = 0; i < 257; ++i) this->c[i] = 0;
	this->ht = NULL;

	this->type = FMDummy1::TYPE_256;
	this->k = 0;
	this->loadFactor = 0.0;
	this->allChars = true;
	this->selectedChars = "all";

	this->textSize = 0;

	this->builder = NULL;
	this->countOperation = NULL;
}

void FMDummy1::freeMemory() {
	for (int i = 0; i < 256; ++i) if (this->bwtWithRanks[i] != NULL) delete[] this->bwtWithRanks[i];
	if (this->alignedBWTWithRanks != NULL) delete[] this->alignedBWTWithRanks;
	if (this->ordChars != NULL) delete[] this->ordChars;
	if (this->ht != NULL) delete this->ht;
}

void FMDummy1::build(unsigned char* text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->freeMemory();
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
	} else this->ordChars = breakByDelimeter(this->selectedChars, '.', this->ordCharsLen);
	unsigned int bwtLen;
	unsigned char *bwt = NULL;
	if (this->k > 1) {
		this->ht = new HT(this->k, this->loadFactor);
		unsigned int saLen;
		unsigned int *sa = getSA(text, textLen, saLen, 0, this->verbose);
		if (this->verbose) cout << "Creating hash table ... " << flush;
		if (this->allChars) this->ht->buildWithEntries(text, textLen, sa, saLen);
		else this->ht->buildWithEntries(text, textLen, sa, saLen, this->ordChars, this->ordCharsLen);
		if (this->verbose) cout << "Done" << endl;
		bwt = getBWT(text, textLen, sa, saLen, bwtLen, this->verbose);
		delete[] sa;
	} else bwt = getBWT(text, textLen, bwtLen, this->verbose);
	if (this->verbose) cout << "Compacting BWT for selected chars ... " << flush;
	++bwtLen;
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

	fillArrayC(text, textLen, this->c, verbose);

	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	this->alignedBWTWithRanks = builder(bwtDenseInLong, bwtDenseInLongLen, this->ordChars, this->ordCharsLen, this->bwtWithRanks, this->bwtWithRanksLen);
	if (this->verbose) cout << "Done" << endl;

	for (unsigned int i = 0; i < this->ordCharsLen; ++i) delete[] bwtDenseInLong[this->ordChars[i]];
	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummy1::getIndexSize() {
	unsigned int size = sizeof(this->ordCharsLen) + sizeof(this->bwtWithRanksLen) + sizeof(this->type) + sizeof(this->k) + sizeof(this->loadFactor) + sizeof(this->allChars) + this->selectedChars.size();
	size += (257 * sizeof(unsigned int) + 256 * sizeof(unsigned long long*));
	if (this->ordCharsLen > 0) {
		size += (256 * sizeof(unsigned long long*));
		size += (this->ordCharsLen * sizeof(unsigned char) + this->ordCharsLen * this->bwtWithRanksLen * sizeof(unsigned long long));
	}
	if (this->ht != NULL) size += this->ht->getHTSize();
	return size;
}

unsigned int FMDummy1::getTextSize() {
	return this->textSize;
}

unsigned int FMDummy1::count(unsigned char *pattern, unsigned int patternLen) {
	return (this->*countOperation)(pattern, patternLen);
}

unsigned int FMDummy1::count_std_256_counter48(unsigned char *pattern, unsigned int patternLen) {
	return count_256_counter48(pattern, patternLen - 1, this->c, this->alignedBWTWithRanks, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1]);
}

unsigned int FMDummy1::count_std_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	return count_512_counter40(pattern, patternLen - 1, this->c, this->alignedBWTWithRanks, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1]);
}

unsigned int FMDummy1::count_hash_256_counter48(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_256_counter48(pattern, patternLen - this->ht->k, this->c, this->alignedBWTWithRanks, leftBoundary + 1, rightBoundary);
	} else {
		return this->count_std_256_counter48(pattern, patternLen);
	}
}

unsigned int FMDummy1::count_hash_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_512_counter40(pattern, patternLen - this->ht->k, this->c, this->alignedBWTWithRanks, leftBoundary + 1, rightBoundary);
	} else {
		return this->count_std_512_counter40(pattern, patternLen);
	}
}

unsigned int *FMDummy1::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummy1::save(char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	bool nullPointer = false;
	bool notNullPointer = true;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	unsigned int selectedCharsLen = this->selectedChars.size();
	fwrite(&selectedCharsLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->selectedChars.c_str(), (size_t)sizeof(char), (size_t)selectedCharsLen, outFile);
	fwrite(&this->k, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->loadFactor, (size_t)sizeof(double), (size_t)1, outFile);
	fwrite(&this->ordCharsLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(&this->bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->ordCharsLen > 0) {
		fwrite(this->ordChars, (size_t)sizeof(unsigned int), (size_t)this->ordCharsLen, outFile);
		for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
			fwrite(this->alignedBWTWithRanks[this->ordChars[i]], (size_t)sizeof(unsigned long long), (size_t)(this->bwtWithRanksLen - 16), outFile);
		}
	}
	if (this->ht == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	else {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		this->ht->save(outFile);
	}
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummy1::load(char *fileName) {
	this->free();
	bool isNotNullPointer;
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	result = fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	unsigned int selectedCharsLen;
	result = fread(&selectedCharsLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	char *selectedCharsTemp = new char[selectedCharsLen + 1];
	result = fread(selectedCharsTemp, (size_t)sizeof(char), (size_t)selectedCharsLen, inFile);
	if (result != selectedCharsLen) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	selectedCharsTemp[selectedCharsLen] = '\0';
	this->selectedChars = selectedCharsTemp;
	delete [] selectedCharsTemp;
	if (this->selectedChars == "all") this->allChars = true;
	else this->allChars = false;
	result = fread(&this->k, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->loadFactor, (size_t)sizeof(double), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->ordCharsLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	if (result != 257) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->ordCharsLen > 0) {
		this->ordChars = new unsigned int[this->ordCharsLen];
		result = fread(this->ordChars, (size_t)sizeof(unsigned int), (size_t)this->ordCharsLen, inFile);
		if (result != this->ordCharsLen) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
		this->alignedBWTWithRanks = new unsigned long long*[256];
		for (int i = 0; i < 256; ++i) this->alignedBWTWithRanks[i] = NULL;
		for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
			unsigned int c = this->ordChars[i];
			this->bwtWithRanks[c] = new unsigned long long[this->bwtWithRanksLen];
			this->alignedBWTWithRanks[c] = this->bwtWithRanks[c];
			while ((unsigned long long)(this->alignedBWTWithRanks[c]) % 128) ++(this->alignedBWTWithRanks[c]);
			result = fread(this->alignedBWTWithRanks[c], (size_t)sizeof(unsigned long long), (size_t)(this->bwtWithRanksLen - 16), inFile);
			if (result != (this->bwtWithRanksLen - 16)) {
				cout << "Error loading index from " << fileName << endl;
				exit(1);
			}
		}
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->ht = new HT();
		this->ht->load(inFile);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

/*FMDUMMY2*/

void FMDummy2::setType(string indexType, string schema) {
	if (indexType == "512") this->type = FMDummy2::TYPE_512;
	else if (indexType == "256") this->type = FMDummy2::TYPE_256;
	else {
		cout << "Error: not valid index type" << endl;
		exit(1);
	}
	if (schema == "CB") this->schema = FMDummy2::SCHEMA_CB;
	else if (schema == "SCBO") this->schema = FMDummy2::SCHEMA_SCBO;
	else {
		cout << "Error: not valid index schema" << endl;
		exit(1);
	}
}

void FMDummy2::setBitsPerChar(string bitsPerChar) {
	if (bitsPerChar == "3") this->bitsPerChar = FMDummy2::BITS_3;
	else this->bitsPerChar = FMDummy2::BITS_4;
}

void FMDummy2::setMaxEncodedCharsLen() {
	this->maxEncodedCharsLen = 0;
	for (int i = 0; i < 256; ++i) if (this->encodedCharsLen[i] > this->maxEncodedCharsLen) this->maxEncodedCharsLen = this->encodedCharsLen[i];
}

void FMDummy2::setK(unsigned int k) {
	if (k < 2) {
		cout << "Error: not valid k value" << endl;
		exit(1);
	}
	this->k = k;
}

void FMDummy2::setLoadFactor(double loadFactor) {
	if (loadFactor <= 0.0 || loadFactor >= 1.0) {
		cout << "Error: not valid loadFactor value" << endl;
		exit(1);
	}
	this->loadFactor = loadFactor;
}

void FMDummy2::setFunctions() {
	if (this->k > 1) {
		switch (this->type) {
		case FMDummy2::TYPE_512:
			this->builder = &buildRank_512_counter40;
			switch (this->schema) {
			case FMDummy2::SCHEMA_CB:
				this->countOperation = &FMDummy2::count_hash_CB_512_counter40;
				break;
			case FMDummy2::SCHEMA_SCBO:
				this->countOperation = &FMDummy2::count_hash_SCBO_512_counter40;
				break;
			default:
				cout << "Error: not valid index schema" << endl;
				exit(1);
			}
			break;
		case FMDummy2::TYPE_256:
			this->builder = &buildRank_256_counter48;
			switch (this->schema) {
			case FMDummy2::SCHEMA_CB:
				this->countOperation = &FMDummy2::count_hash_CB_256_counter48;
				break;
			case FMDummy2::SCHEMA_SCBO:
				this->countOperation = &FMDummy2::count_hash_SCBO_256_counter48;
				break;
			default:
				cout << "Error: not valid index schema" << endl;
				exit(1);
			}
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	} else {
		switch (this->type) {
		case FMDummy2::TYPE_512:
			this->builder = &buildRank_512_counter40;
			switch (this->schema) {
			case FMDummy2::SCHEMA_CB:
				this->countOperation = &FMDummy2::count_std_CB_512_counter40;
				break;
			case FMDummy2::SCHEMA_SCBO:
				this->countOperation = &FMDummy2::count_std_SCBO_512_counter40;
				break;
			default:
				cout << "Error: not valid index schema" << endl;
				exit(1);
			}
			break;
		case FMDummy2::TYPE_256:
			this->builder = &buildRank_256_counter48;
			switch (this->schema) {
			case FMDummy2::SCHEMA_CB:
				this->countOperation = &FMDummy2::count_std_CB_256_counter48;
				break;
			case FMDummy2::SCHEMA_SCBO:
				this->countOperation = &FMDummy2::count_std_SCBO_256_counter48;
				break;
			default:
				cout << "Error: not valid index schema" << endl;
				exit(1);
			}
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	}
}

void FMDummy2::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy2::initialize() {
	for (int i = 0; i < 256; ++i) this->bwtWithRanks[i] = NULL;
	this->alignedBWTWithRanks = NULL;
	this->bwtWithRanksLen = 0;
	for (int i = 0; i < 256; ++i) this->encodedChars[i] = NULL;
	for (int i = 0; i < 256; ++i) this->encodedCharsLen[i] = 0;
	this->maxEncodedCharsLen = 0;
	for (int i = 0; i < 257; ++i) this->c[i] = 0;
	this->bInC = 0;
	this->ht = NULL;

	this->type = FMDummy2::TYPE_256;
	this->schema = FMDummy2::SCHEMA_SCBO;
	this->bitsPerChar = FMDummy2::BITS_4;
	this->k = 0;
	this->loadFactor = 0.0;

	this->textSize = 0;

	this->builder = NULL;
	this->countOperation = NULL;
}

void FMDummy2::freeMemory() {
	for (int i = 0; i < 256; ++i) if (this->bwtWithRanks[i] != NULL) delete[] this->bwtWithRanks[i];
	for (int i = 0; i < 256; ++i) if (this->encodedChars[i] != NULL) delete[] this->encodedChars[i];
	if (this->alignedBWTWithRanks != NULL) delete[] this->alignedBWTWithRanks;
	if (this->ht != NULL) delete this->ht;
}

void FMDummy2::build(unsigned char *text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->freeMemory();
	this->textSize = textLen;
	if (this->k > 1) {
		this->ht = new HT(this->k, this->loadFactor);
		unsigned int saLen;
		unsigned int *sa = getSA(text, textLen, saLen, 0, this->verbose);
		if (this->verbose) cout << "Creating hash table ... " << flush;
		this->ht->buildWithEntries(text, textLen, sa, saLen);
		if (this->verbose) cout << "Done" << endl;
		delete[] sa;
	}
	unsigned int encodedTextLen;
	unsigned char *encodedText = NULL;
	unsigned int b = 0;
	switch (this->schema) {
	case FMDummy2::SCHEMA_SCBO:
		if (this->verbose) cout << "SCBO text encoding ... " << flush;
		encodedText = getEncodedInSCBO(this->bitsPerChar, text, textLen, encodedTextLen, this->encodedChars, this->encodedCharsLen);
		if (this->verbose) cout << "Done" << endl;
		break;
	case FMDummy2::SCHEMA_CB:
		if (this->verbose) cout << "CB text encoding ... " << flush;
		encodedText = getEncodedInCB(this->bitsPerChar, text, textLen, encodedTextLen, this->encodedChars, this->encodedCharsLen, b);
		if (this->verbose) cout << "Done" << endl;
		break;
	}
	this->setMaxEncodedCharsLen();
	unsigned int bwtLen;
	unsigned int encodedSALen;
	unsigned int *encodedSA = getSA(encodedText, encodedTextLen, encodedSALen, 0, this->verbose);
	unsigned char *bwt = getBWT(encodedText, encodedTextLen, encodedSA, encodedSALen, bwtLen, this->verbose);
	if (this->ht == NULL) delete[] encodedSA;
	unsigned int ordCharsLen = (unsigned int)exp2((double)this->bitsPerChar);
	if (this->verbose) cout << "Compacting BWT ... " << flush;
	++bwtLen;
	unsigned int bwtDenseLen = (bwtLen / 8);
	if (bwtLen % 8 > 0) ++bwtDenseLen;
	unsigned int bwtDenseInLongLen = bwtDenseLen / sizeof(unsigned long long);
	if (bwtDenseLen % sizeof(unsigned long long) > 0) ++bwtDenseInLongLen;
	unsigned long long *bwtDenseInLong[256];
	unsigned int *ordChars = new unsigned int[ordCharsLen];
	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		ordChars[i] = i + 1;
		unsigned char *bwtDense = getBinDenseForChar(bwt, bwtLen, ordChars[i]);
		bwtDenseInLong[ordChars[i]] = new unsigned long long[bwtDenseInLongLen + 8];
		for (unsigned long long j = 0; j < bwtDenseInLongLen; ++j) {
			bwtDenseInLong[ordChars[i]][j] = ((unsigned long long)bwtDense[8 * j + 7] << 56) | ((unsigned long long)bwtDense[8 * j + 6] << 48) | ((unsigned long long)bwtDense[8 * j + 5] << 40) | ((unsigned long long)bwtDense[8 * j + 4] << 32) | ((unsigned long long)bwtDense[8 * j + 3] << 24) | ((unsigned long long)bwtDense[8 * j + 2] << 16) | ((unsigned long long)bwtDense[8 * j + 1] << 8) | (unsigned long long)bwtDense[8 * j];
		}
		for (unsigned long long j = bwtDenseInLongLen; j < bwtDenseInLongLen + 8; ++j) {
			bwtDenseInLong[ordChars[i]][j] = 0ULL;
		}
		delete[] bwtDense;
	}
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;

	fillArrayC(encodedText, encodedTextLen, this->c, verbose);
	if (this->schema == FMDummy2::SCHEMA_CB) this->bInC = this->c[b];

	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	this->alignedBWTWithRanks = builder(bwtDenseInLong, bwtDenseInLongLen, ordChars, ordCharsLen, this->bwtWithRanks, this->bwtWithRanksLen);
	if (this->verbose) cout << "Done" << endl;
	if (this->ht != NULL)  {
		if (this->verbose) cout << "Modifying hash table for encoded text ... " << flush;
		for (unsigned int i = 0; i < this->ht->bucketsNum; ++i) {
			if (this->ht->alignedBoundariesHT[2 * i] != HT::emptyValueHT) {
				unsigned int encodedPatternLen;
				unsigned char *encodedPattern = encode(this->ht->alignedEntriesHT + (i * this->ht->k), this->ht->k, this->encodedChars, this->encodedCharsLen, encodedPatternLen);
				switch (this->schema) {
				case FMDummy2::SCHEMA_SCBO:
					switch(this->type) {
					case FMDummy2::TYPE_256:
						getCountBoundaries_256_counter48(encodedPattern, encodedPatternLen - 1, this->c, this->alignedBWTWithRanks, this->c[encodedPattern[encodedPatternLen - 1]] + 1, this->c[encodedPattern[encodedPatternLen - 1] + 1], this->ht->alignedBoundariesHT[2 * i], this->ht->alignedBoundariesHT[2 * i + 1]);
						break;
					case FMDummy2::TYPE_512:
						getCountBoundaries_512_counter40(encodedPattern, encodedPatternLen - 1, this->c, this->alignedBWTWithRanks, this->c[encodedPattern[encodedPatternLen - 1]] + 1, this->c[encodedPattern[encodedPatternLen - 1] + 1], this->ht->alignedBoundariesHT[2 * i], this->ht->alignedBoundariesHT[2 * i + 1]);
						break;
					}
					break;
				case FMDummy2::SCHEMA_CB:
					switch(this->type) {
					case FMDummy2::TYPE_256:
						getCountBoundaries_256_counter48(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, 1, this->bInC, this->ht->alignedBoundariesHT[2 * i], this->ht->alignedBoundariesHT[2 * i + 1]);
						break;
					case FMDummy2::TYPE_512:
						getCountBoundaries_512_counter40(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, 1, this->bInC, this->ht->alignedBoundariesHT[2 * i], this->ht->alignedBoundariesHT[2 * i + 1]);
						break;
					}
					break;
				}
				delete[] encodedPattern;
			}
		}
		unsigned char lutPattern[3];
		lutPattern[2] = '\0';
		for (int i = 0; i < 256; ++i) {
			lutPattern[0] = (unsigned char)i;
			for (int j = 0; j < 256; ++j) {
				lutPattern[1] = (unsigned char)j;
				unsigned int encodedPatternLen;
				unsigned char *encodedLutPattern = encode(lutPattern, 2, this->encodedChars, this->encodedCharsLen, encodedPatternLen);
				binarySearch(encodedSA, encodedText, 0, encodedSALen, encodedLutPattern, encodedPatternLen, this->ht->lut2[i][j][0], this->ht->lut2[i][j][1]);
				++this->ht->lut2[i][j][1];
				delete[] encodedLutPattern;
			}
		}
		if (this->verbose) cout << "Done" << endl;
		delete[] encodedSA;
	}

	for (unsigned int i = 0; i < ordCharsLen; ++i) delete[] bwtDenseInLong[ordChars[i]];
	delete[] ordChars;
	delete[] encodedText;

	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummy2::getIndexSize() {
	unsigned int encodedCharsLenSum = 0;
	for (int i = 0; i < 256; ++i) {
		encodedCharsLenSum += this->encodedCharsLen[i];
	}
	unsigned int size = (sizeof(this->type) + sizeof(this->schema) + sizeof(this->bitsPerChar) + sizeof(this->k) + sizeof(this->loadFactor) + sizeof(this->maxEncodedCharsLen) + sizeof(bInC) + sizeof(this->bwtWithRanksLen));
	size += (257 * sizeof(unsigned int) + 256 * sizeof(unsigned long long *) + 256 * sizeof(unsigned int) + 256 * sizeof(unsigned char *));
	size += ((unsigned int)exp2((double)this->bitsPerChar) * this->bwtWithRanksLen * sizeof(unsigned long long) + encodedCharsLenSum * sizeof(unsigned char));
	if (this->ht != NULL) size += this->ht->getHTSize();
	return size;
}

unsigned int FMDummy2::getTextSize() {
	return this->textSize;
}

unsigned int FMDummy2::count(unsigned char *pattern, unsigned int patternLen) {
	return (this->*countOperation)(pattern, patternLen);
}

unsigned int FMDummy2::count_std_SCBO_256_counter48(unsigned char *pattern, unsigned int patternLen) {
	bool wrongEncoding = false;
	unsigned int encodedPatternLen;
	unsigned char *encodedPattern = encodePattern(pattern, patternLen, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
	unsigned int count;
	if (wrongEncoding) count = 0;
	else count = count_256_counter48(encodedPattern, encodedPatternLen - 1, this->c, this->alignedBWTWithRanks, this->c[encodedPattern[encodedPatternLen - 1]] + 1, this->c[encodedPattern[encodedPatternLen - 1] + 1]);
	delete[] encodedPattern;
	return count;
}

unsigned int FMDummy2::count_std_CB_256_counter48(unsigned char *pattern, unsigned int patternLen) {
	bool wrongEncoding = false;
	unsigned int encodedPatternLen;
	unsigned char *encodedPattern = encodePattern(pattern, patternLen, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
	unsigned int count;
	if (wrongEncoding) count = 0;
	else count = count_256_counter48(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, 1, this->bInC);
	delete[] encodedPattern;
	return count;
}

unsigned int FMDummy2::count_std_SCBO_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	bool wrongEncoding = false;
	unsigned int encodedPatternLen;
	unsigned char *encodedPattern = encodePattern(pattern, patternLen, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
	unsigned int count;
	if (wrongEncoding) count = 0;
	else count = count_512_counter40(encodedPattern, encodedPatternLen - 1, this->c, this->alignedBWTWithRanks, this->c[encodedPattern[encodedPatternLen - 1]] + 1, this->c[encodedPattern[encodedPatternLen - 1] + 1]);
	delete[] encodedPattern;
	return count;
}

unsigned int FMDummy2::count_std_CB_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	bool wrongEncoding = false;
	unsigned int encodedPatternLen;
	unsigned char *encodedPattern = encodePattern(pattern, patternLen, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
	unsigned int count;
	if (wrongEncoding) count = 0;
	else count = count_512_counter40(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, 1, this->bInC);
	delete[] encodedPattern;
	return count;
}

unsigned int FMDummy2::count_hash_SCBO_256_counter48(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		bool wrongEncoding = false;
		unsigned int encodedPatternLen;
		unsigned char *encodedPattern = encodePattern(pattern, patternLen - this->ht->k, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
		unsigned int count;
		if (wrongEncoding) count = 0;
		else count = count_256_counter48(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, leftBoundary + 1, rightBoundary);
		delete[] encodedPattern;
		return count;
	} else {
		return this->count_std_SCBO_256_counter48(pattern, patternLen);
	}
}

unsigned int FMDummy2::count_hash_CB_256_counter48(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		bool wrongEncoding = false;
		unsigned int encodedPatternLen;
		unsigned char *encodedPattern = encodePattern(pattern, patternLen - this->ht->k, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
		unsigned int count;
		if (wrongEncoding) count = 0;
		else count = count_256_counter48(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, leftBoundary + 1, rightBoundary);
		delete[] encodedPattern;
		return count;
	} else {
		return this->count_std_CB_256_counter48(pattern, patternLen);
	}
}

unsigned int FMDummy2::count_hash_SCBO_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		bool wrongEncoding = false;
		unsigned int encodedPatternLen;
		unsigned char *encodedPattern = encodePattern(pattern, patternLen - this->ht->k, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
		unsigned int count;
		if (wrongEncoding) count = 0;
		else count = count_512_counter40(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, leftBoundary + 1, rightBoundary);
		delete[] encodedPattern;
		return count;
	} else {
		return this->count_std_SCBO_512_counter40(pattern, patternLen);
	}
}

unsigned int FMDummy2::count_hash_CB_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		bool wrongEncoding = false;
		unsigned int encodedPatternLen;
		unsigned char *encodedPattern = encodePattern(pattern, patternLen - this->ht->k, this->encodedChars, this->encodedCharsLen, this->maxEncodedCharsLen, encodedPatternLen, wrongEncoding);
		unsigned int count;
		if (wrongEncoding) count = 0;
		else count = count_512_counter40(encodedPattern, encodedPatternLen, this->c, this->alignedBWTWithRanks, leftBoundary + 1, rightBoundary);
		delete[] encodedPattern;
		return count;
	} else {
		return this->count_std_CB_512_counter40(pattern, patternLen);
	}
}

unsigned int *FMDummy2::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummy2::save(char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	bool nullPointer = false;
	bool notNullPointer = true;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->schema, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->bitsPerChar, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->k, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->loadFactor, (size_t)sizeof(double), (size_t)1, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(this->encodedCharsLen, (size_t)sizeof(unsigned int), (size_t)256, outFile);
	for (int i = 0; i < 256; ++i) {
		if (this->encodedChars[i] != NULL) fwrite(this->encodedChars[i], (size_t)sizeof(unsigned char), (size_t)this->encodedCharsLen[i], outFile);
	}
	unsigned int maxChar = (unsigned int)exp2((double)this->bitsPerChar);
	fwrite(&this->bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->bwtWithRanksLen > 0) {
		for (unsigned int i = 1; i < maxChar + 1; ++i) {
			fwrite(this->alignedBWTWithRanks[i], (size_t)sizeof(unsigned long long), (size_t)(this->bwtWithRanksLen - 16), outFile);
		}
	}
	if (this->schema == FMDummy2::SCHEMA_CB) fwrite(&this->bInC, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->ht == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	else {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		this->ht->save(outFile);
	}
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummy2::load(char *fileName) {
	this->free();
	bool isNotNullPointer;
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	result = fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->schema, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->bitsPerChar, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->k, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->loadFactor, (size_t)sizeof(double), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	if (result != 257) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->encodedCharsLen, (size_t)sizeof(unsigned int), (size_t)256, inFile);
	if (result != 256) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	this->setMaxEncodedCharsLen();
	for (int i = 0; i < 256; ++i) {
		if (this->encodedCharsLen[i] == 0) continue;
		this->encodedChars[i] = new unsigned char[this->encodedCharsLen[i]];
		result = fread(this->encodedChars[i], (size_t)sizeof(unsigned char), (size_t)this->encodedCharsLen[i], inFile);
		if (result != this->encodedCharsLen[i]) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
	unsigned int maxChar = (unsigned int)exp2((double)this->bitsPerChar);
	result = fread(&this->bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->bwtWithRanksLen > 0) {
		this->alignedBWTWithRanks = new unsigned long long*[256];
		for (int i = 0; i < 256; ++i) this->alignedBWTWithRanks[i] = NULL;
		for (unsigned int i = 1; i < maxChar + 1; ++i) {
			this->bwtWithRanks[i] = new unsigned long long[this->bwtWithRanksLen];
			this->alignedBWTWithRanks[i] = this->bwtWithRanks[i];
			while ((unsigned long long)(this->alignedBWTWithRanks[i]) % 128) ++(this->alignedBWTWithRanks[i]);
			result = fread(this->alignedBWTWithRanks[i], (size_t)sizeof(unsigned long long), (size_t)(this->bwtWithRanksLen - 16), inFile);
			if (result != (this->bwtWithRanksLen - 16)) {
				cout << "Error loading index from " << fileName << endl;
				exit(1);
			}
		}
	}
	if (this->schema == FMDummy2::SCHEMA_CB) {
		result = fread(&this->bInC, (size_t)sizeof(unsigned int), (size_t)1, inFile);
		if (result != 1) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->ht = new HT();
		this->ht->load(inFile);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;

}

/*FMDUMMY3*/

void FMDummy3::setType(string indexType) {
	if (indexType == "1024") this->type = FMDummy3::TYPE_1024;
	else if (indexType == "512") this->type = FMDummy3::TYPE_512;
	else {
		cout << "Error: not valid index type" << endl;
		exit(1);
	}
}

void FMDummy3::setK(unsigned int k) {
	if (k < 2) {
		cout << "Error: not valid k value" << endl;
		exit(1);
	}
	this->k = k;
}

void FMDummy3::setLoadFactor(double loadFactor) {
	if (loadFactor <= 0.0 || loadFactor >= 1.0) {
		cout << "Error: not valid loadFactor value" << endl;
		exit(1);
	}
	this->loadFactor = loadFactor;
}

void FMDummy3::setFunctions() {
	if (this->k > 1) {
		switch (this->type) {
		case FMDummy3::TYPE_1024:
			this->countOperation = &FMDummy3::count_hash_1024_enc125;
			break;
		case FMDummy3::TYPE_512:
			this->countOperation = &FMDummy3::count_hash_512_enc125;
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	} else {
		switch (this->type) {
		case FMDummy3::TYPE_1024:
			this->countOperation = &FMDummy3::count_std_1024_enc125;
			break;
		case FMDummy3::TYPE_512:
			this->countOperation = &FMDummy3::count_std_512_enc125;
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	}
}

void FMDummy3::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy3::initialize() {
	this->bwtWithRanks = NULL;
	this->alignedBWTWithRanks = NULL;
	this->bwtWithRanksLen = 0;
	for (int i = 0; i < 257; ++i) this->c[i] = 0;
	this->ht = NULL;

	this->type = FMDummy3::TYPE_512;
	this->k = 0;
	this->loadFactor = 0.0;

	this->textSize = 0;

	this->countOperation = NULL;
}

void FMDummy3::freeMemory() {
	if (this->bwtWithRanks != NULL) delete[] this->bwtWithRanks;
	if (this->ht != NULL) delete this->ht;
}

void FMDummy3::buildRank_512_enc125(unsigned char *bwtEnc125, unsigned int bwtLen) {
	unsigned int rank[4] = {0, 0, 0, 0};
	unsigned char *p, signs[4] = { 'A', 'C', 'G', 'T' };

	unsigned int *resRank[4];
	for (int i = 0; i < 4; ++i) {
		resRank[i] = new unsigned int[(bwtLen * 8) / 384 + 1];
		resRank[i][0] = 0;
	}
	for (int s = 0; s < 4; ++s) {
		p = bwtEnc125;
		for (unsigned int i = 8; p < bwtEnc125 + bwtLen; ++p, i += 8) {
			rank[s] += this->lut[signs[s]][*p];
			if (i % 384 == 0) resRank[s][i / 384] = rank[s];
		}
	}
	this->bwtWithRanksLen = bwtLen + 4 * 4 * ((bwtLen * 8) / 384 + 1) + 128;
	this->bwtWithRanks = new unsigned char[this->bwtWithRanksLen];
	this->alignedBWTWithRanks = this->bwtWithRanks;
	while ((unsigned long long)this->alignedBWTWithRanks % 128) ++(this->alignedBWTWithRanks);
	p = bwtEnc125;
	unsigned int counter = 0;
	for (unsigned int i = 0; p < bwtEnc125 + bwtLen; ++p, ++i) {
		if (i % 48 == 0) {
			for (int s = 0; s < 4; ++s) {
				this->alignedBWTWithRanks[counter++] = (resRank[s][i / 48] & 0x000000FFU);
				this->alignedBWTWithRanks[counter++] = ((resRank[s][i / 48] & 0x0000FF00U) >> 8);
				this->alignedBWTWithRanks[counter++] = ((resRank[s][i / 48] & 0x00FF0000U) >> 16);
				this->alignedBWTWithRanks[counter++] = ((resRank[s][i / 48] & 0xFF000000U) >> 24);
			}
		}
		this->alignedBWTWithRanks[counter++] = *p;
	}
	for (int i = 0; i < 4; ++i) delete[] resRank[i];
}

void FMDummy3::buildRank_1024_enc125(unsigned char *bwtEnc125, unsigned int bwtLen) {
	unsigned int rank[4] = {0, 0, 0, 0};
	unsigned char *p, signs[4] = { 'A', 'C', 'G', 'T' };

	unsigned int *resRank[4];
	for (int i = 0; i < 4; ++i) {
		resRank[i] = new unsigned int[(bwtLen * 8) / 896 + 1];
		resRank[i][0] = 0;
	}
	for (int s = 0; s < 4; ++s) {
		p = bwtEnc125;
		for (unsigned int i = 8; p < bwtEnc125 + bwtLen; ++p, i += 8) {
			rank[s] += this->lut[signs[s]][*p];
			if (i % 896 == 0) resRank[s][i / 896] = rank[s];
		}
	}
	this->bwtWithRanksLen = bwtLen + 4 * 4 * ((bwtLen * 8) / 896 + 1) + 128;
	this->bwtWithRanks = new unsigned char[this->bwtWithRanksLen];
	this->alignedBWTWithRanks = this->bwtWithRanks;
	while ((unsigned long long)this->alignedBWTWithRanks % 128) ++(this->alignedBWTWithRanks);
	p = bwtEnc125;
	unsigned int counter = 0;
	for (unsigned int i = 0; p < bwtEnc125 + bwtLen; ++p, ++i) {
		if (i % 112 == 0) {
			for (int s = 0; s < 4; ++s) {
				this->alignedBWTWithRanks[counter++] = (resRank[s][i / 112] & 0x000000FFU);
				this->alignedBWTWithRanks[counter++] = ((resRank[s][i / 112] & 0x0000FF00U) >> 8);
				this->alignedBWTWithRanks[counter++] = ((resRank[s][i / 112] & 0x00FF0000U) >> 16);
				this->alignedBWTWithRanks[counter++] = ((resRank[s][i / 112] & 0xFF000000U) >> 24);
			}
		}
		this->alignedBWTWithRanks[counter++] = *p;
	}
	for (int i = 0; i < 4; ++i) delete[] resRank[i];
}

void FMDummy3::build(unsigned char *text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->freeMemory();
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
	unsigned char *bwt = NULL;
	unsigned int selectedOrdChars[4] = { (unsigned int)'A', (unsigned int)'C', (unsigned int)'G', (unsigned int)'T' };
	if (this->k > 1) {
		this->ht = new HT(this->k, this->loadFactor);
		unsigned int saLen;
		unsigned int *sa = getSA(convertedText, textLen, saLen, 0, this->verbose);
		if (this->verbose) cout << "Creating hash table ... " << flush;

		this->ht->buildWithEntries(convertedText, textLen, sa, saLen, selectedOrdChars, 4);
		if (this->verbose) cout << "Done" << endl;
		bwt = getBWT(convertedText, textLen, sa, saLen, bwtLen, this->verbose);
		delete[] sa;
	} else bwt = getBWT(convertedText, textLen, bwtLen, this->verbose);
	if (this->verbose) cout << "Encoding BWT ... " << flush;
	++bwtLen;
	unsigned int bwtEnc125Len;
	unsigned char *bwtEnc125 = encode125(bwt, bwtLen, selectedOrdChars, bwtEnc125Len);
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;
	fill125LUT(selectedOrdChars, this->lut);
	fillArrayC(convertedText, textLen, this->c, verbose);
	delete[] convertedText;
	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	switch (this->type) {
	case FMDummy3::TYPE_512:
		this->buildRank_512_enc125(bwtEnc125, bwtEnc125Len);
		break;
	case FMDummy3::TYPE_1024:
		this->buildRank_1024_enc125(bwtEnc125, bwtEnc125Len);
		break;
	}
	if (this->verbose) cout << "Done" << endl;
	delete[] bwtEnc125;
	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummy3::getIndexSize() {
	unsigned int size = sizeof(this->type) + sizeof(this->k) + sizeof(this->loadFactor) + sizeof(this->bwtWithRanksLen);
	size += (257 * sizeof(unsigned int) + sizeof(unsigned char*) + 256 * 125 * sizeof(unsigned int));
	size += this->bwtWithRanksLen * sizeof(unsigned char);
	if (this->ht != NULL) size += this->ht->getHTSize();
	return size;
}

unsigned int FMDummy3::getTextSize() {
	return this->textSize;
}

unsigned int FMDummy3::count(unsigned char *pattern, unsigned int patternLen) {
	return (this->*countOperation)(pattern, patternLen);
}

unsigned int FMDummy3::count_std_512_enc125(unsigned char *pattern, unsigned int patternLen) {
	return count_512_enc125(pattern, patternLen - 1, this->c, this->alignedBWTWithRanks, this->lut, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1]);
}

unsigned int FMDummy3::count_std_1024_enc125(unsigned char *pattern, unsigned int patternLen) {
	return count_1024_enc125(pattern, patternLen - 1, this->c, this->alignedBWTWithRanks, this->lut, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1]);
}

unsigned int FMDummy3::count_hash_512_enc125(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_512_enc125(pattern, patternLen - this->ht->k, this->c, this->alignedBWTWithRanks, this->lut, leftBoundary + 1, rightBoundary);
	} else {
		return this->count_std_512_enc125(pattern, patternLen);
	}
}

unsigned int FMDummy3::count_hash_1024_enc125(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_1024_enc125(pattern, patternLen - this->ht->k, this->c, this->alignedBWTWithRanks, this->lut, leftBoundary + 1, rightBoundary);
	} else {
		return this->count_std_1024_enc125(pattern, patternLen);
	}
}

unsigned int *FMDummy3::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummy3::save(char *fileName) {
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	bool nullPointer = false;
	bool notNullPointer = true;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->k, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->loadFactor, (size_t)sizeof(double), (size_t)1, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(this->lut, (size_t)sizeof(unsigned int), (size_t)(256 * 125), outFile);
	fwrite(&this->bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->bwtWithRanksLen > 0) fwrite(this->alignedBWTWithRanks, (size_t)sizeof(unsigned char), (size_t)(this->bwtWithRanksLen - 128), outFile);
	if (this->ht == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	else {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		this->ht->save(outFile);
	}
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummy3::load(char *fileName) {
	this->free();
	bool isNotNullPointer;
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	result = fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->k, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->loadFactor, (size_t)sizeof(double), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	if (result != 257) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->lut, (size_t)sizeof(unsigned int), (size_t)(256 * 125), inFile);
	if (result != (256 * 125)) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->bwtWithRanksLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->bwtWithRanksLen > 0) {
		this->bwtWithRanks = new unsigned char[this->bwtWithRanksLen];
		this->alignedBWTWithRanks = this->bwtWithRanks;
		while ((unsigned long long)this->alignedBWTWithRanks % 128) ++this->alignedBWTWithRanks;
		result = fread(this->alignedBWTWithRanks, (size_t)sizeof(unsigned char), (size_t)(this->bwtWithRanksLen - 128), inFile);
		if (result != (this->bwtWithRanksLen - 128)) {
			cout << "Error loading index from " << fileName << endl;
			exit(1);
		}
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->ht = new HT();
		this->ht->load(inFile);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

/*FMDUMMYWT*/

void FMDummyWT::setType(string wtType, string indexType) {
	if (wtType == "2") this->wtType = FMDummyWT::TYPE_WT2;
	else if (wtType == "4") this->wtType = FMDummyWT::TYPE_WT4;
	else if (wtType == "8") this->wtType = FMDummyWT::TYPE_WT8;
	else {
		cout << "Error: not valid WT type" << endl;
		exit(1);
	}
	if (indexType == "512") this->type = FMDummyWT::TYPE_512;
	else if (indexType == "1024") this->type = FMDummyWT::TYPE_1024;
	else {
		cout << "Error: not valid index type" << endl;
		exit(1);
	}
}

void FMDummyWT::setK(unsigned int k) {
	if (k < 2) {
		cout << "Error: not valid k value" << endl;
		exit(1);
	}
	this->k = k;
}

void FMDummyWT::setLoadFactor(double loadFactor) {
	if (loadFactor <= 0.0 || loadFactor >= 1.0) {
		cout << "Error: not valid loadFactor value" << endl;
		exit(1);
	}
	this->loadFactor = loadFactor;
}

void FMDummyWT::setFunctions() {
	if (this->k > 1) {
		switch (this->wtType) {
		case FMDummyWT::TYPE_WT2:
			switch (this->type) {
			case FMDummyWT::TYPE_512:
				this->countOperation = &FMDummyWT::count_WT2hash_512_counter40;
				break;
			case FMDummyWT::TYPE_1024:
				this->countOperation = &FMDummyWT::count_WT2hash_1024_counter32;
				break;
			default:
				cout << "Error: not valid WT type" << endl;
				exit(1);
			}
			break;
		case FMDummyWT::TYPE_WT4:
			switch (this->type) {
			case FMDummyWT::TYPE_512:
				this->countOperation = &FMDummyWT::count_WT4hash_512;
				break;
			case FMDummyWT::TYPE_1024:
				this->countOperation = &FMDummyWT::count_WT4hash_1024;
				break;
			default:
				cout << "Error: not valid WT type" << endl;
				exit(1);
			}
			break;
		case FMDummyWT::TYPE_WT8:
			switch (this->type) {
			case FMDummyWT::TYPE_512:
				this->countOperation = &FMDummyWT::count_WT8hash_512;
				break;
			case FMDummyWT::TYPE_1024:
				this->countOperation = &FMDummyWT::count_WT8hash_1024;
				break;
			default:
				cout << "Error: not valid WT type" << endl;
				exit(1);
			}
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	} else {
		switch (this->wtType) {
		case FMDummyWT::TYPE_WT2:
			switch (this->type) {
			case FMDummyWT::TYPE_512:
				this->countOperation = &FMDummyWT::count_WT2std_512_counter40;
				break;
			case FMDummyWT::TYPE_1024:
				this->countOperation = &FMDummyWT::count_WT2std_1024_counter32;
				break;
			default:
				cout << "Error: not valid WT type" << endl;
				exit(1);
			}
			break;
		case FMDummyWT::TYPE_WT4:
			switch (this->type) {
			case FMDummyWT::TYPE_512:
				this->countOperation = &FMDummyWT::count_WT4std_512;
				break;
			case FMDummyWT::TYPE_1024:
				this->countOperation = &FMDummyWT::count_WT4std_1024;
				break;
			default:
				cout << "Error: not valid WT type" << endl;
				exit(1);
			}
			break;
		case FMDummyWT::TYPE_WT8:
			switch (this->type) {
			case FMDummyWT::TYPE_512:
				this->countOperation = &FMDummyWT::count_WT8std_512;
				break;
			case FMDummyWT::TYPE_1024:
				this->countOperation = &FMDummyWT::count_WT8std_1024;
				break;
			default:
				cout << "Error: not valid WT type" << endl;
				exit(1);
			}
			break;
		default:
			cout << "Error: not valid index type" << endl;
			exit(1);
		}
	}
}

void FMDummyWT::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummyWT::initialize() {
	this->wt = NULL;
	this->ht = NULL;

	for (int i = 0; i < 256; ++i) {
		this->code[i] = 0;
		this->codeLen[i] = 0;
	}
	for (int i = 0; i < 257; ++i) this->c[i] = 0;

	this->wtType = FMDummyWT::TYPE_WT2;
	this->type = FMDummyWT::TYPE_512;
	this->k = 0;
	this->loadFactor = 0.0;

	this->textSize = 0;

	this->countOperation = NULL;
}

void FMDummyWT::freeMemory() {
	if (this->wt != NULL) delete this->wt;
	if (this->ht != NULL) delete this->ht;
}

WT *FMDummyWT::createWT2_512_counter40(unsigned char *text, unsigned int textLen, unsigned int wtLevel) {
	if (textLen == 0) return NULL;

	unsigned int textLengths[2];
	unsigned char *texts[2];
	for (int i = 0; i < 2; ++i) {
		textLengths[i] = 0;
		texts[i] = new unsigned char[textLen];
	}

	bool childExists = false;
	for (unsigned int i = 0; i < textLen; ++i) {
		if (this->codeLen[text[i]] > wtLevel) {
			childExists = true;
			int nextNode = (this->code[text[i]] >> wtLevel) & 1;
			texts[nextNode][textLengths[nextNode]++] = text[i];
		}
	}

	if (!childExists) return NULL;

	WT *node = new WT(2);

	for (int i = 0; i < 2; ++i) {
		node->nodes[i] = this->createWT2_512_counter40(texts[i], textLengths[i], wtLevel + 1);
		delete[] texts[i];
	}

	unsigned long long nodeBitLenTemp = textLen / 64;
	if (textLen % 64 > 0) ++nodeBitLenTemp;
	node->bitsLen = nodeBitLenTemp / 7;
	if (nodeBitLenTemp % 7 > 0) ++(node->bitsLen);
	node->bitsLen *= 8;
	if (textLen % 448 == 0) ++(node->bitsLen);
	node->bitsLen += 16;
	node->bits = new unsigned long long[node->bitsLen];
	node->alignedBits = node->bits;
	while ((unsigned long long)(node->alignedBits) % 128) ++(node->alignedBits);
	unsigned long long ranks[4];
	for (int i = 0; i < 4; ++i) ranks[i] = 0;
	unsigned long long rank = 0;
	int currRank = 0, j = 0;
	unsigned long long packedBits = 0;
	long long packedBitsCounter = 0, lastRankCounter = 0;
	for (unsigned int i = 0; i < textLen; ++i, ++j) {
		if (j % 128 == 0) ranks[currRank++] = rank;
		if (i % 448 == 0) {
			if (j > 0) node->alignedBits[lastRankCounter] += ((ranks[1] - ranks[0]) << 56) + ((ranks[2] - ranks[1]) << 48) + ((ranks[3] - ranks[2]) << 40);
			lastRankCounter = packedBitsCounter;
			node->alignedBits[packedBitsCounter++] = rank;
			ranks[0] = rank;
			currRank = 1;
			j = 0;
		}
		int nextNode = (this->code[text[i]] >> wtLevel) & 1;
		if (nextNode == 1) ++rank;
		packedBits = (packedBits << 1) + nextNode;
		if (i % 64 == 63) {
			node->alignedBits[packedBitsCounter++] = packedBits;
			packedBits = 0;
		}
	}
	if (textLen % 64 != 0) {
		for (int i = textLen % 64; i % 64 != 0; ++i) {
			packedBits = (packedBits << 1);
		}
		node->alignedBits[packedBitsCounter++] = packedBits;
	}
	if (j > 0) {
		ranks[currRank++] = rank;
		for (int i = currRank; i < 4; ++i) ranks[i] = ranks[i - 1];
		node->alignedBits[lastRankCounter] += ((ranks[1] - ranks[0]) << 56) + ((ranks[2] - ranks[1]) << 48) + ((ranks[3] - ranks[2]) << 40);
	}
	if (textLen % 448 == 0) {
		node->alignedBits[packedBitsCounter++] = rank;
	}
	return node;
}

WT *FMDummyWT::createWT2_1024_counter32(unsigned char *text, unsigned int textLen, unsigned int wtLevel) {
	if (textLen == 0) return NULL;

	unsigned int textLengths[2];
	unsigned char *texts[2];
	for (int i = 0; i < 2; ++i) {
		textLengths[i] = 0;
		texts[i] = new unsigned char[textLen];
	}

	bool childExists = false;
	for (unsigned int i = 0; i < textLen; ++i) {
		if (this->codeLen[text[i]] > wtLevel) {
			childExists = true;
			int nextNode = (this->code[text[i]] >> wtLevel) & 1;
			texts[nextNode][textLengths[nextNode]++] = text[i];
		}
	}

	if (!childExists) return NULL;

	WT *node = new WT(2);

	for (int i = 0; i < 2; ++i) {
		node->nodes[i] = createWT2_1024_counter32(texts[i], textLengths[i], wtLevel + 1);
		delete[] texts[i];
	}

	unsigned long long nodeBitLenTemp = textLen / 64;
	if (textLen % 64 > 0) ++nodeBitLenTemp;
	node->bitsLen = nodeBitLenTemp / 15;
	if (nodeBitLenTemp % 15 > 0) ++(node->bitsLen);
	node->bitsLen *= 16;
	if (textLen % 960 == 0) ++(node->bitsLen);
	node->bitsLen += 16;
	node->bits = new unsigned long long[node->bitsLen];
	node->alignedBits = node->bits;
	while ((unsigned long long)(node->alignedBits) % 128) ++(node->alignedBits);
	unsigned long long ranks[6];
	for (int i = 0; i < 6; ++i) ranks[i] = 0;
	unsigned long long rank = 0;
	int currRank = 0, j = 0;
	unsigned long long packedBits = 0;
	long long packedBitsCounter = 0, lastRankCounter = 0;
	for (unsigned int i = 0; i < textLen; ++i, ++j) {
		if (j % 192 == 0) ranks[currRank++] = rank;
		if (i % 960 == 0) {
			if (j > 0) node->alignedBits[lastRankCounter] += ((ranks[1] - ranks[0]) << 56) + ((ranks[2] - ranks[1]) << 48) + ((ranks[3] - ranks[2]) << 40) + ((ranks[4] - ranks[3]) << 32);
			lastRankCounter = packedBitsCounter;
			node->alignedBits[packedBitsCounter++] = rank;
			ranks[0] = rank;
			currRank = 1;
			j = 0;
		}
		int nextNode = (this->code[text[i]] >> wtLevel) & 1;
		if (nextNode == 1) ++rank;
		packedBits = (packedBits << 1) + nextNode;
		if (i % 64 == 63) {
			node->alignedBits[packedBitsCounter++] = packedBits;
			packedBits = 0;
		}
	}
	if (textLen % 64 != 0) {
		for (int i = textLen % 64; i % 64 != 0; ++i) {
			packedBits = (packedBits << 1);
		}
		node->alignedBits[packedBitsCounter++] = packedBits;
	}
	if (j > 0) {
		ranks[currRank++] = rank;
		for (int i = currRank; i < 6; ++i) ranks[i] = ranks[i - 1];
		node->alignedBits[lastRankCounter] += ((ranks[1] - ranks[0]) << 56) + ((ranks[2] - ranks[1]) << 48) + ((ranks[3] - ranks[2]) << 40) + ((ranks[4] - ranks[3]) << 32);
	}
	if (textLen % 960 == 0) {
		node->alignedBits[packedBitsCounter++] = rank;
	}
	return node;
}

WT *FMDummyWT::createWT4(unsigned char *text, unsigned int textLen, unsigned int wtLevel){
	if (textLen == 0) return NULL;

	unsigned int textLengths[4];
	unsigned int rank[4];
	unsigned char *texts[4];
	for (int i = 0; i < 4; ++i) {
		textLengths[i] = 0;
		rank[i] = 0;
		texts[i] = new unsigned char[textLen];
	}

	bool childExists = false;
	for (unsigned int i = 0; i < textLen; ++i) {
		if (this->codeLen[text[i]] > wtLevel) {
			childExists = true;
			int nextNode = (this->code[text[i]] >> (2 * wtLevel)) & 3;
			texts[nextNode][textLengths[nextNode]++] = text[i];
		}
	}

	if (!childExists) return NULL;

	WT *node = new WT(4);

	for (int i = 0; i < 4; ++i) {
		node->nodes[i] = this->createWT4(texts[i], textLengths[i], wtLevel + 1);
		delete[] texts[i];
	}

	unsigned long long nodeDibitLenTemp = (2 * textLen) / 64;
	if ((2 * textLen) % 64 > 0) ++nodeDibitLenTemp;
	node->bitsLen = nodeDibitLenTemp / (this->type - 2);
	if (nodeDibitLenTemp % (this->type - 2) > 0) ++(node->bitsLen);
	node->bitsLen *= this->type;
	if (textLen % ((this->type - 2) * 32) == 0) node->bitsLen += 2;
	node->bitsLen += 16;
	node->bits = new unsigned long long[node->bitsLen];
	node->alignedBits = node->bits;
	while ((unsigned long long)(node->alignedBits) % 128) ++(node->alignedBits);
	unsigned long long packedDibits = 0;
	long long packedDibitsCounter = 0;
	for (unsigned int i = 0; i < textLen; ++i) {
		if (i % ((this->type - 2) * 32) == 0) {
			for (int j = 0; j < 2; ++j) {
				node->alignedBits[packedDibitsCounter++] = ((unsigned long long)rank[2 * j] << 32) + rank[2 * j + 1];
			}
		}
		int nextNode = (this->code[text[i]] >> (2 * wtLevel)) & 3;
		++rank[nextNode];
		packedDibits = (packedDibits << 2) + nextNode;
		if (i % 32 == 31) {
			node->alignedBits[packedDibitsCounter++] = packedDibits;
			packedDibits = 0;
		}
	}
	if (textLen % 32 != 0) {
		for (int i = textLen % 32; i % 32 != 0; ++i) {
			packedDibits = (packedDibits << 2);
		}
		node->alignedBits[packedDibitsCounter++] = packedDibits;
	}
	if (textLen % ((this->type - 2) * 32) == 0) {
		for (int j = 0; j < 2; ++j) {
			node->alignedBits[packedDibitsCounter++] = ((unsigned long long)rank[2 * j] << 32) + rank[2 * j + 1];
		}
	}
	return node;
}
WT *FMDummyWT::createWT8(unsigned char *text, unsigned int textLen, unsigned int wtLevel) {
	if (textLen == 0) return NULL;

	unsigned int textLengths[8];
	unsigned int rank[8];
	unsigned char *texts[8];
	for (int i = 0; i < 8; ++i) {
		textLengths[i] = 0;
		rank[i] = 0;
		texts[i] = new unsigned char[textLen];
	}

	bool childExists = false;
	for (unsigned int i = 0; i < textLen; ++i) {
		if (this->codeLen[text[i]] > wtLevel) {
			childExists = true;
			int nextNode = (this->code[text[i]] >> (3 * wtLevel)) & 7;
			texts[nextNode][textLengths[nextNode]++] = text[i];
		}
	}

	if (!childExists) return NULL;

	WT *node = new WT(8);

	for (int i = 0; i < 8; ++i) {
		node->nodes[i] = this->createWT8(texts[i], textLengths[i], wtLevel + 1);
		delete[] texts[i];
	}

	unsigned long long nodeTripleLenTemp = (3 * textLen) / 63;
	if ((3 * textLen) % 63 > 0) ++nodeTripleLenTemp;
	node->bitsLen = nodeTripleLenTemp / (this->type - 4);
	if (nodeTripleLenTemp % (this->type - 4) > 0) ++(node->bitsLen);
	node->bitsLen *= this->type;
	if (textLen % ((this->type - 4) * 21) == 0) node->bitsLen += 4;
	node->bitsLen += 16;
	node->bits = new unsigned long long[node->bitsLen];
	node->alignedBits = node->bits;
	while ((unsigned long long)(node->alignedBits) % 128) ++(node->alignedBits);
	unsigned long long packedTriples = 0;
	long long packedTripleCounter = 0;
	for (unsigned int i = 0; i < textLen; ++i) {
		if (i % ((this->type - 4) * 21) == 0) {
			for (int j = 0; j < 4; ++j) {
				node->alignedBits[packedTripleCounter++] = ((unsigned long long)rank[2 * j] << 32) + rank[2 * j + 1];
			}
		}
		int nextNode = (this->code[text[i]] >> (3 * wtLevel)) & 7;
		++rank[nextNode];
		packedTriples = (packedTriples << 3) + nextNode;
		if (i % 21 == 20) {
			node->alignedBits[packedTripleCounter++] = packedTriples;
			packedTriples = 0;
		}
	}
	if (textLen % 21 != 0) {
		for (int i = textLen % 21; i % 21 != 0; ++i) {
			packedTriples = (packedTriples << 3);
		}
		node->alignedBits[packedTripleCounter++] = packedTriples;
	}
	if (textLen % ((this->type - 4) * 21) == 0) {
		for (int j = 0; j < 4; ++j) {
			node->alignedBits[packedTripleCounter++] = ((unsigned long long)rank[2 * j] << 32) + rank[2 * j + 1];
		}
	}
	return node;
}

void FMDummyWT::build(unsigned char *text, unsigned int textLen) {
	checkNullChar(text, textLen);
	this->freeMemory();
	this->textSize = textLen;
	unsigned int bwtLen;
	unsigned char *bwt = NULL;
	if (this->k > 1) {
		this->ht = new HT(this->k, this->loadFactor);
		unsigned int saLen;
		unsigned int *sa = getSA(text, textLen, saLen, 0, this->verbose);
		if (this->verbose) cout << "Creating hash table ... " << flush;
		this->ht->buildWithEntries(text, textLen, sa, saLen);
		if (this->verbose) cout << "Done" << endl;
		bwt = getBWT(text, textLen, sa, saLen, bwtLen, this->verbose);
		delete[] sa;
	} else bwt = getBWT(text, textLen, bwtLen, this->verbose);
	if (this->verbose) cout << "Huffman encoding ... " << flush;
	encodeHuff(this->wtType, bwt, bwtLen, this->code, this->codeLen);
	if (this->verbose) cout << "Done" << endl;
	if (this->verbose) cout << "Creating WT ... " << flush;
	switch (this->wtType) {
	case FMDummyWT::TYPE_WT2:
		switch (this->type) {
		case FMDummyWT::TYPE_512:
			this->wt = this->createWT2_512_counter40(bwt, bwtLen, 0);
			break;
		case FMDummyWT::TYPE_1024:
			this->wt = this->createWT2_1024_counter32(bwt, bwtLen, 0);
			break;
		}
		break;
	case FMDummyWT::TYPE_WT4:
		this->wt = this->createWT4(bwt, bwtLen, 0);
		break;
	case FMDummyWT::TYPE_WT8:
		this->wt = this->createWT8(bwt, bwtLen, 0);
		break;
	}
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;
	fillArrayC(text, textLen, this->c, verbose);
	if (this->verbose) cout << "Index successfully built" << endl;
}

unsigned int FMDummyWT::getIndexSize() {
	unsigned int size = sizeof(this->type) + sizeof(this->wtType) + sizeof(this->k) + sizeof(this->loadFactor) + sizeof(WT *) + sizeof(HT *);
	size += (257 * sizeof(unsigned int) + 256 * sizeof(unsigned int) + 256 * sizeof(unsigned long long));
	if (this->wt != NULL) size += this->wt->getWTSize();
	if (this->ht != NULL) size += this->ht->getHTSize();
	return size;
}

unsigned int FMDummyWT::getTextSize() {
	return this->textSize;
}

unsigned int FMDummyWT::count(unsigned char *pattern, unsigned int patternLen) {
	return (this->*countOperation)(pattern, patternLen);
}

unsigned int *FMDummyWT::locate(unsigned char *pattern, unsigned int patternLen) {
	return 0;
}

void FMDummyWT::save(char *fileName) {
	bool nullPointer = false;
	bool notNullPointer = true;
	if (this->verbose) cout << "Saving index in " << fileName << " ... " << flush;
	FILE *outFile;
	outFile = fopen(fileName, "w");
	fwrite(&this->verbose, (size_t)sizeof(bool), (size_t)1, outFile);
	fwrite(&this->wtType, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->type, (size_t)sizeof(int), (size_t)1, outFile);
	fwrite(&this->k, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(&this->loadFactor, (size_t)sizeof(double), (size_t)1, outFile);
	fwrite(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	fwrite(this->c, (size_t)sizeof(unsigned int), (size_t)257, outFile);
	fwrite(this->code, (size_t)sizeof(unsigned long long), (size_t)256, outFile);
	fwrite(this->codeLen, (size_t)sizeof(unsigned int), (size_t)256, outFile);
	if (this->wt == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	else {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		this->wt->save(outFile);
	}
	if (this->ht == NULL) fwrite(&nullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
	else {
		fwrite(&notNullPointer, (size_t)sizeof(bool), (size_t)1, outFile);
		this->ht->save(outFile);
	}
	fclose(outFile);
	if (this->verbose) cout << "Done" << endl;
}

void FMDummyWT::load(char *fileName) {
	this->free();
	bool isNotNullPointer;
	FILE *inFile;
	inFile = fopen(fileName, "rb");
	size_t result;
	result = fread(&this->verbose, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (this->verbose) cout << "Loading index from " << fileName << " ... " << flush;
	result = fread(&this->wtType, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->type, (size_t)sizeof(int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->k, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->loadFactor, (size_t)sizeof(double), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&this->textSize, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->c, (size_t)sizeof(unsigned int), (size_t)257, inFile);
	if (result != 257) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->code, (size_t)sizeof(unsigned long long), (size_t)256, inFile);
	if (result != 256) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(this->codeLen, (size_t)sizeof(unsigned int), (size_t)256, inFile);
	if (result != 256) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->wt = new WT();
		this->wt->load(inFile);
	}
	result = fread(&isNotNullPointer, (size_t)sizeof(bool), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index from " << fileName << endl;
		exit(1);
	}
	if (isNotNullPointer) {
		this->ht = new HT();
		this->ht->load(inFile);
	}
	fclose(inFile);
	this->setFunctions();
	if (this->verbose) cout << "Done" << endl;
}

unsigned int FMDummyWT::count_WT2std_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	return count_WT2_512_counter40(pattern, patternLen - 1, this->c, this->wt, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1], this->code, this->codeLen);
}

unsigned int FMDummyWT::count_WT2std_1024_counter32(unsigned char *pattern, unsigned int patternLen) {
	return count_WT2_1024_counter32(pattern, patternLen - 1, this->c, this->wt, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1], this->code, this->codeLen);
}

unsigned int FMDummyWT::count_WT4std_512(unsigned char *pattern, unsigned int patternLen) {
	return count_WT4_512(pattern, patternLen - 1, this->c, this->wt, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1], this->code, this->codeLen);
}

unsigned int FMDummyWT::count_WT4std_1024(unsigned char *pattern, unsigned int patternLen) {
	return count_WT4_1024(pattern, patternLen - 1, this->c, this->wt, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1], this->code, this->codeLen);
}

unsigned int FMDummyWT::count_WT8std_512(unsigned char *pattern, unsigned int patternLen) {
	return count_WT8_512(pattern, patternLen - 1, this->c, this->wt, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1], this->code, this->codeLen);
}

unsigned int FMDummyWT::count_WT8std_1024(unsigned char *pattern, unsigned int patternLen) {
	return count_WT8_1024(pattern, patternLen - 1, this->c, this->wt, this->c[pattern[patternLen - 1]] + 1, this->c[pattern[patternLen - 1] + 1], this->code, this->codeLen);
}

unsigned int FMDummyWT::count_WT2hash_512_counter40(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_WT2_512_counter40(pattern, patternLen - this->ht->k, this->c, this->wt, leftBoundary + 1, rightBoundary, this->code, this->codeLen);
	} else {
		return this->count_WT2std_512_counter40(pattern, patternLen);
	}
}

unsigned int FMDummyWT::count_WT2hash_1024_counter32(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_WT2_1024_counter32(pattern, patternLen - this->ht->k, this->c, this->wt, leftBoundary + 1, rightBoundary, this->code, this->codeLen);
	} else {
		return this->count_WT2std_1024_counter32(pattern, patternLen);
	}
}

unsigned int FMDummyWT::count_WT4hash_512(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_WT4_512(pattern, patternLen - this->ht->k, this->c, this->wt, leftBoundary + 1, rightBoundary, this->code, this->codeLen);
	} else {
		return this->count_WT4std_512(pattern, patternLen);
	}
}

unsigned int FMDummyWT::count_WT4hash_1024(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_WT4_1024(pattern, patternLen - this->ht->k, this->c, this->wt, leftBoundary + 1, rightBoundary, this->code, this->codeLen);
	} else {
		return this->count_WT4std_1024(pattern, patternLen);
	}
}

unsigned int FMDummyWT::count_WT8hash_512(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_WT8_512(pattern, patternLen - this->ht->k, this->c, this->wt, leftBoundary + 1, rightBoundary, this->code, this->codeLen);
	} else {
		return this->count_WT8std_512(pattern, patternLen);
	}
}

unsigned int FMDummyWT::count_WT8hash_1024(unsigned char *pattern, unsigned int patternLen) {
	if (this->ht->k <= patternLen) {
		unsigned int leftBoundary, rightBoundary;
		this->ht->getBoundariesWithEntries(pattern + (patternLen - this->ht->k), leftBoundary, rightBoundary);
		return count_WT8_1024(pattern, patternLen - this->ht->k, this->c, this->wt, leftBoundary + 1, rightBoundary, this->code, this->codeLen);
	} else {
		return this->count_WT8std_1024(pattern, patternLen);
	}
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

unsigned long long** buildRank_256_counter48(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned long long** bwtWithRanks, unsigned int &bwtWithRanksLen) {
	unsigned long long *p, pops, rank, b1, b2;
	unsigned long long **alignedBWTWithRanks = new unsigned long long*[256];
	for (int i = 0; i < 256; ++i) alignedBWTWithRanks[i] = NULL;
	bwtWithRanksLen = (bwtInLongLen + (bwtInLongLen * 64) / 192 + 1 + 16);

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		unsigned int c = ordChars[i];
		unsigned long long *resRank = new unsigned long long[(bwtInLongLen * 64) / 192 + 1];
		p = bwtInLong[c];
		rank = 0;
		pops = 0;
		for (unsigned int j = 0; p < bwtInLong[c] + bwtInLongLen; p += 3, ++j) {
			pops = __builtin_popcountll(*p);
			b1 = (pops << 56);
			pops += __builtin_popcountll(*(p + 1));
			b2 = (pops << 48);
			pops += __builtin_popcountll(*(p + 2));
			resRank[j] = rank + b1 + b2;
			rank += pops;
			pops = 0;
		}
		bwtWithRanks[c] = new unsigned long long[bwtWithRanksLen];
		alignedBWTWithRanks[c] = bwtWithRanks[c];
		while ((unsigned long long)alignedBWTWithRanks[c] % 128) ++(alignedBWTWithRanks[c]);
		p = bwtInLong[c];
		unsigned int counter = 0;
		for (unsigned int j = 0; p < bwtInLong[c] + bwtInLongLen; ++p, ++j) {
			if (j % 3 == 0) alignedBWTWithRanks[c][counter++] = resRank[j / 3];
			alignedBWTWithRanks[c][counter++] = *p;
		}
		delete[] resRank;
	}
	return alignedBWTWithRanks;
}

unsigned long long** buildRank_512_counter40(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned long long** bwtWithRanks, unsigned int &bwtWithRanksLen) {
	unsigned long long *p, pop1, pop2, pop3, rank, b1, b2, b3;
	unsigned long long **alignedBWTWithRanks = new unsigned long long*[256];
	for (int i = 0; i < 256; ++i) alignedBWTWithRanks[i] = NULL;
	bwtWithRanksLen = (bwtInLongLen + (bwtInLongLen * 64) / 448 + 1 + 16);

	for (unsigned int i = 0; i < ordCharsLen; ++i) {
		unsigned int c = ordChars[i];
		unsigned long long *resRank = new unsigned long long[(bwtInLongLen * 64) / 448 + 1];
		p = bwtInLong[c];
		rank = 0;
		for (unsigned int j = 0; p < bwtInLong[c] + bwtInLongLen; p += 7, ++j) {
			pop1 = __builtin_popcountll(*p) + __builtin_popcountll(*(p + 1));
			b1 = (pop1 << 56);
			pop2 = __builtin_popcountll(*(p + 2)) + __builtin_popcountll(*(p + 3));
			b2 = (pop2 << 48);
			pop3 = __builtin_popcountll(*(p + 4)) + __builtin_popcountll(*(p + 5));
			b3 = (pop3 << 40);
			resRank[j] = rank + b1 + b2 + b3;
			rank += pop1 + pop2 + pop3 + __builtin_popcountll(*(p + 6));
		}
		bwtWithRanks[c] = new unsigned long long[bwtWithRanksLen];
		alignedBWTWithRanks[c] = bwtWithRanks[c];
		while ((unsigned long long)alignedBWTWithRanks[c] % 128) ++(alignedBWTWithRanks[c]);
		p = bwtInLong[c];
		unsigned int counter = 0;
		for (unsigned int j = 0; p < bwtInLong[c] + bwtInLongLen; ++p, ++j) {
			if (j % 7 == 0) alignedBWTWithRanks[c][counter++] = resRank[j / 7];
			alignedBWTWithRanks[c][counter++] = *p;
		}
		delete[] resRank;
	}
	return alignedBWTWithRanks;
}

unsigned int getRank_256_counter48(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
	unsigned int j = i / 192;
	unsigned long long *p = bwtWithRanks[c] + 4 * j;
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
		if (bwtWithRanks[c] == NULL) return 0;
		firstVal = C[c] + getRank_256_counter48(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 4 * ((firstVal - 1) / 192), 0, 3);
		lastVal = C[c] + getRank_256_counter48(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 4 * (lastVal / 192), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		if (bwtWithRanks[c] == NULL) return 0;
		firstVal = C[c] + getRank_256_counter48(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_256_counter48(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

void getCountBoundaries_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal, unsigned int &leftBoundary, unsigned int &rightBoundary) {
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

	leftBoundary = firstVal - 1;
	rightBoundary = lastVal;
}

unsigned int getRank_512_counter40(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {

	unsigned int j = i / 448;
	unsigned long long *p = bwtWithRanks[c] + 8 * j;
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
		if (bwtWithRanks[c] == NULL) return 0;
		firstVal = C[c] + getRank_512_counter40(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 8 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRank_512_counter40(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[pattern[i - 2]] + 8 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		if (bwtWithRanks[c] == NULL) return 0;
		firstVal = C[c] + getRank_512_counter40(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_512_counter40(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

void getCountBoundaries_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal, unsigned int &leftBoundary, unsigned int &rightBoundary) {
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

	leftBoundary = firstVal - 1;
	rightBoundary = lastVal;
}

bool sortCharsCount(unsigned int* i, unsigned int* j) {
	return (i[1] > j[1]);
}

unsigned char *getEncodedInSCBO(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen) {

	int max = (int)exp2((double)bits);

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

	for (int o = 1; o < max - 2; ++o) {
		for (int b = 1; b < max - 2; ++b) {
			for (int c = 1; c < max - 2; ++c) {
				for (int s = 1; s < max - 2; ++s) {
					if (o + b + c + s != max) continue;
					int sig = charsLen;
					unsigned int total = 0;
					unsigned int curr = 0;
					unsigned int upperBound = o;
					unsigned int symbolLen = 1;
					if (sig > 0) {
						for (unsigned int i = 0; i < upperBound; ++i) {
							total += charsCountVector[curr][1] * symbolLen;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= upperBound;
						upperBound = b * s;
						++symbolLen;
					}
					while (sig > 0) {
						for (unsigned int i = 0; i < upperBound; ++i) {
							total += charsCountVector[curr][1] * symbolLen;
							++curr;
							if (curr >= charsLen) break;
						}
						sig -= upperBound;
						upperBound *= c;
						++symbolLen;
					}
					if (total < totalTotal) {
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

	unsigned int o = best[0];
	unsigned int b = best[1];
	unsigned int s = best[2];
	unsigned int c = best[3];

	unsigned int bStart = o;
	unsigned int sStart = bStart + b;
	unsigned int cStart = sStart + s;

	for (unsigned int i = 0; i < charsLen; ++i) {
		if (i < o) {
			encodedChars[chars[i]] = new unsigned char[1];
			encodedChars[chars[i]][0] = i + 1;
			encodedCharsLen[chars[i]] = 1;
			continue;
		}
		if (i < o + b * s) {
			int j = i - o;
			encodedChars[chars[i]] = new unsigned char[2];
			encodedChars[chars[i]][0] = bStart + j / s + 1;
			encodedChars[chars[i]][1] = sStart + j % s + 1;
			encodedCharsLen[chars[i]] = 2;
			continue;
		}
		unsigned int temp1 = b * s;
		unsigned int symbolLen = 3;
		unsigned int temp2 = 0;
		while (true) {
			temp2 = b * s * (unsigned int)pow((double)c, (double)(symbolLen - 2));
			if (i < o + temp1 + temp2) {
				int j = i - o - temp1;
				encodedChars[chars[i]] = new unsigned char[symbolLen];
				encodedChars[chars[i]][0] = bStart + j / (s * (unsigned int)pow((double)c, (double)(symbolLen - 2))) + 1;
				for (unsigned int k = 1; k < symbolLen - 1; ++k) {
					encodedChars[chars[i]][k] = cStart + (j / (s * (unsigned int)pow((double)c, (double)(symbolLen - 2 - k)))) % c + 1;
				}
				encodedChars[chars[i]][symbolLen - 1] = sStart + j % s + 1;
				encodedCharsLen[chars[i]] = symbolLen;
				break;
			}
			temp1 += temp2;
			++symbolLen;
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

	int max = (int)exp2((double)bits);

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

	for (int b = 1; b < max; ++b) {
		for (int c = 1; c < max; ++c) {
			if (b + c != max) continue;
			int sig = charsLen;
			unsigned int total = 0;
			unsigned int curr = 0;
			unsigned int upperBound = b;
			unsigned int symbolLen = 1;
			while (sig > 0) {
				for (unsigned int i = 0; i < upperBound; ++i) {
					total += charsCountVector[curr][1] * symbolLen;
					++curr;
					if (curr >= charsLen) break;
				}
				sig -= upperBound;
				upperBound *= c;
				++symbolLen;
			}
			if (total < totalTotal) {
				totalTotal = total;
				best[0] = b;
				best[1] = c;
			}
		}
	}

	b = best[0];
	unsigned int c = best[1];

	unsigned int bStart = 0;
	unsigned int cStart = b;

	for (unsigned int i = 0; i < charsLen; ++i) {
		if (i < b) {
			encodedChars[chars[i]] = new unsigned char[1];
			encodedChars[chars[i]][0] = i + 1;
			encodedCharsLen[chars[i]] = 1;
			continue;
		}
		unsigned int temp1 = b;
		unsigned int symbolLen = 2;
		unsigned int temp2 = 0;
		while (true) {
			temp2 = b * (unsigned int)pow((double)c, (double)(symbolLen - 1));
			if (i < temp1 + temp2) {
				int j = i - temp1;
				encodedChars[chars[i]] = new unsigned char[symbolLen];
				encodedChars[chars[i]][0] = bStart + j / (unsigned int)pow((double)c, (double)(symbolLen - 1)) + 1;
				for (unsigned int k = 1; k < symbolLen - 1; ++k) {
					encodedChars[chars[i]][k] = cStart + (j / (unsigned int)pow((double)c, (double)(symbolLen - 1 - k))) % c + 1;
				}
				encodedChars[chars[i]][symbolLen - 1] = cStart + j % c + 1;
				encodedCharsLen[chars[i]] = symbolLen;
				break;
			}
			temp1 += temp2;
			++symbolLen;
		}
	}
	++b;

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

unsigned char *encodePattern(unsigned char* pattern, unsigned int patternLen, unsigned char** encodedChars, unsigned int* encodedCharsLen, unsigned int maxEncodedCharsLen, unsigned int &encodedPatternLen, bool &wrongEncoding) {
	unsigned char* encodedPattern = new unsigned char[maxEncodedCharsLen * patternLen + 1];
	unsigned char* p = pattern;
	encodedPatternLen = 0;
	for (; p < pattern + patternLen; ++p) {
		if (encodedCharsLen[*p] == 0) {
			wrongEncoding = true;
			break;
		}
		for (unsigned int i = 0; i < encodedCharsLen[*p]; ++i) encodedPattern[encodedPatternLen++] = encodedChars[*p][i];
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
	case 'A':
		memcpy(&rank, p, (size_t)4);
		break;
	default:
		return 0;
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
	case 'A':
		memcpy(&rank, p, (size_t)4);
		break;
	default:
		return 0;
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

class Symbol {
public:
	unsigned int symbol;
	unsigned int code;
	unsigned int codeLen;

	Symbol(unsigned symbol) {
		this->symbol = symbol;
		this->code = 0;
		this->codeLen = 0;
	}
};

class HuffNode {
public:
	unsigned int freq;
	vector<Symbol> symbols;

	HuffNode() {
		this->freq = 0;
	};

	void add(HuffNode *hNode, unsigned int d, unsigned int order);

	HuffNode(unsigned int freq, unsigned int symbol) {
		this->freq = freq;
		Symbol *sym = new Symbol(symbol);
		this->symbols.push_back(*sym);
	};
	bool operator()(const HuffNode* lh, const HuffNode* rh) const { return lh->freq > rh->freq; }
};

void HuffNode::add(HuffNode *hNode, unsigned int d, unsigned int order) {
	this->freq += hNode->freq;
	for(vector<Symbol>::iterator it = hNode->symbols.begin(); it != hNode->symbols.end(); ++it) {
		Symbol *s = new Symbol((*it).symbol);
		//s->code = (*it).code + order * (unsigned int)pow((double)d, (double)(*it).codeLen);
		s->code = (*it).code * d + order;
		s->codeLen = (*it).codeLen + 1;
		this->symbols.push_back(*s);
	}
}

void encodeHuff(unsigned int d, unsigned char *text, unsigned int textLen, unsigned long long *huffCode, unsigned int *huffCodeLen) {
	unsigned int freq[256];
	for (int i = 0; i < 256; ++i) {
		freq[i] = 0;
		huffCode[i] = 0;
		huffCodeLen[i] = 0;
	}
	for (unsigned int i = 0; i < textLen; ++i) freq[(unsigned int)text[i]]++;
	unsigned int n = 0;
	priority_queue<HuffNode *, vector<HuffNode *>, HuffNode> heap;
	for (int i = 0; i < 256; ++i) if (freq[i] > 0) {
		++n;
		heap.push(new HuffNode(freq[i], (unsigned int)i));
	}
	unsigned int q = (unsigned int)(ceil((double)(n - d) / (d - 1)));
	unsigned int N = d + q * (d - 1);
	for (unsigned int i = 0; i < (N - n); ++i) heap.push(new HuffNode(0, (unsigned int)(256 + i)));
	while (heap.size() > 1) {
		HuffNode *newHuffNode = new HuffNode();
		for (unsigned int i = 0; i < d; ++i) {
			HuffNode *hf = heap.top();
			heap.pop();
			newHuffNode->add(hf, d, i);
			delete hf;
		}
		heap.push(newHuffNode);
	}
	HuffNode *hf = heap.top();
	heap.pop();
	for(vector<Symbol>::iterator it = hf->symbols.begin(); it != hf->symbols.end(); ++it) {
		unsigned int s = (*it).symbol;
		if (s > 255) continue;
		huffCode[s] = (*it).code;
		huffCodeLen[s] = (*it).codeLen;
	}
}

unsigned long long bitMask[64] = {
	0x0000000000000000ULL,
	0x8000000000000000ULL,
	0xC000000000000000ULL,
	0xE000000000000000ULL,
	0xF000000000000000ULL,
	0xF800000000000000ULL,
	0xFC00000000000000ULL,
	0xFE00000000000000ULL,
	0xFF00000000000000ULL,
	0xFF80000000000000ULL,
	0xFFC0000000000000ULL,
	0xFFE0000000000000ULL,
	0xFFF0000000000000ULL,
	0xFFF8000000000000ULL,
	0xFFFC000000000000ULL,
	0xFFFE000000000000ULL,
	0xFFFF000000000000ULL,
	0xFFFF800000000000ULL,
	0xFFFFC00000000000ULL,
	0xFFFFE00000000000ULL,
	0xFFFFF00000000000ULL,
	0xFFFFF80000000000ULL,
	0xFFFFFC0000000000ULL,
	0xFFFFFE0000000000ULL,
	0xFFFFFF0000000000ULL,
	0xFFFFFF8000000000ULL,
	0xFFFFFFC000000000ULL,
	0xFFFFFFE000000000ULL,
	0xFFFFFFF000000000ULL,
	0xFFFFFFF800000000ULL,
	0xFFFFFFFC00000000ULL,
	0xFFFFFFFE00000000ULL,
	0xFFFFFFFF00000000ULL,
	0xFFFFFFFF80000000ULL,
	0xFFFFFFFFC0000000ULL,
	0xFFFFFFFFE0000000ULL,
	0xFFFFFFFFF0000000ULL,
	0xFFFFFFFFF8000000ULL,
	0xFFFFFFFFFC000000ULL,
	0xFFFFFFFFFE000000ULL,
	0xFFFFFFFFFF000000ULL,
	0xFFFFFFFFFF800000ULL,
	0xFFFFFFFFFFC00000ULL,
	0xFFFFFFFFFFE00000ULL,
	0xFFFFFFFFFFF00000ULL,
	0xFFFFFFFFFFF80000ULL,
	0xFFFFFFFFFFFC0000ULL,
	0xFFFFFFFFFFFE0000ULL,
	0xFFFFFFFFFFFF0000ULL,
	0xFFFFFFFFFFFF8000ULL,
	0xFFFFFFFFFFFFC000ULL,
	0xFFFFFFFFFFFFE000ULL,
	0xFFFFFFFFFFFFF000ULL,
	0xFFFFFFFFFFFFF800ULL,
	0xFFFFFFFFFFFFFC00ULL,
	0xFFFFFFFFFFFFFE00ULL,
	0xFFFFFFFFFFFFFF00ULL,
	0xFFFFFFFFFFFFFF80ULL,
	0xFFFFFFFFFFFFFFC0ULL,
	0xFFFFFFFFFFFFFFE0ULL,
	0xFFFFFFFFFFFFFFF0ULL,
	0xFFFFFFFFFFFFFFF8ULL,
	0xFFFFFFFFFFFFFFFCULL,
	0xFFFFFFFFFFFFFFFEULL
};

unsigned long long dibitMask[32] = {
	0x0000000000000000ULL,
	0xC000000000000000ULL,
	0xF000000000000000ULL,
	0xFC00000000000000ULL,
	0xFF00000000000000ULL,
	0xFFC0000000000000ULL,
	0xFFF0000000000000ULL,
	0xFFFC000000000000ULL,
	0xFFFF000000000000ULL,
	0xFFFFC00000000000ULL,
	0xFFFFF00000000000ULL,
	0xFFFFFC0000000000ULL,
	0xFFFFFF0000000000ULL,
	0xFFFFFFC000000000ULL,
	0xFFFFFFF000000000ULL,
	0xFFFFFFFC00000000ULL,
	0xFFFFFFFF00000000ULL,
	0xFFFFFFFFC0000000ULL,
	0xFFFFFFFFF0000000ULL,
	0xFFFFFFFFFC000000ULL,
	0xFFFFFFFFFF000000ULL,
	0xFFFFFFFFFFC00000ULL,
	0xFFFFFFFFFFF00000ULL,
	0xFFFFFFFFFFFC0000ULL,
	0xFFFFFFFFFFFF0000ULL,
	0xFFFFFFFFFFFFC000ULL,
	0xFFFFFFFFFFFFF000ULL,
	0xFFFFFFFFFFFFFC00ULL,
	0xFFFFFFFFFFFFFF00ULL,
	0xFFFFFFFFFFFFFFC0ULL,
	0xFFFFFFFFFFFFFFF0ULL,
	0xFFFFFFFFFFFFFFFCULL
};

unsigned long long interlacedMask2[64] = {
	0x0000000000000000ULL,
	0x4000000000000000ULL,
	0x5000000000000000ULL,
	0x5400000000000000ULL,
	0x5500000000000000ULL,
	0x5540000000000000ULL,
	0x5550000000000000ULL,
	0x5554000000000000ULL,
	0x5555000000000000ULL,
	0x5555400000000000ULL,
	0x5555500000000000ULL,
	0x5555540000000000ULL,
	0x5555550000000000ULL,
	0x5555554000000000ULL,
	0x5555555000000000ULL,
	0x5555555400000000ULL,
	0x5555555500000000ULL,
	0x5555555540000000ULL,
	0x5555555550000000ULL,
	0x5555555554000000ULL,
	0x5555555555000000ULL,
	0x5555555555400000ULL,
	0x5555555555500000ULL,
	0x5555555555540000ULL,
	0x5555555555550000ULL,
	0x5555555555554000ULL,
	0x5555555555555000ULL,
	0x5555555555555400ULL,
	0x5555555555555500ULL,
	0x5555555555555540ULL,
	0x5555555555555550ULL,
	0x5555555555555554ULL,
	0x5555555555555555ULL,
	0xD555555555555555ULL,
	0xF555555555555555ULL,
	0xFD55555555555555ULL,
	0xFF55555555555555ULL,
	0xFFD5555555555555ULL,
	0xFFF5555555555555ULL,
	0xFFFD555555555555ULL,
	0xFFFF555555555555ULL,
	0xFFFFD55555555555ULL,
	0xFFFFF55555555555ULL,
	0xFFFFFD5555555555ULL,
	0xFFFFFF5555555555ULL,
	0xFFFFFFD555555555ULL,
	0xFFFFFFF555555555ULL,
	0xFFFFFFFD55555555ULL,
	0xFFFFFFFF55555555ULL,
	0xFFFFFFFFD5555555ULL,
	0xFFFFFFFFF5555555ULL,
	0xFFFFFFFFFD555555ULL,
	0xFFFFFFFFFF555555ULL,
	0xFFFFFFFFFFD55555ULL,
	0xFFFFFFFFFFF55555ULL,
	0xFFFFFFFFFFFD5555ULL,
	0xFFFFFFFFFFFF5555ULL,
	0xFFFFFFFFFFFFD555ULL,
	0xFFFFFFFFFFFFF555ULL,
	0xFFFFFFFFFFFFFD55ULL,
	0xFFFFFFFFFFFFFF55ULL,
	0xFFFFFFFFFFFFFFD5ULL,
	0xFFFFFFFFFFFFFFF5ULL,
	0xFFFFFFFFFFFFFFFDULL,
};

unsigned long long interlacedMask3[63] = {
	0x0000000000000000ULL,
	0x1000000000000000ULL,
	0x1200000000000000ULL,
	0x1240000000000000ULL,
	0x1248000000000000ULL,
	0x1249000000000000ULL,
	0x1249200000000000ULL,
	0x1249240000000000ULL,
	0x1249248000000000ULL,
	0x1249249000000000ULL,
	0x1249249200000000ULL,
	0x1249249240000000ULL,
	0x1249249248000000ULL,
	0x1249249249000000ULL,
	0x1249249249200000ULL,
	0x1249249249240000ULL,
	0x1249249249248000ULL,
	0x1249249249249000ULL,
	0x1249249249249200ULL,
	0x1249249249249240ULL,
	0x1249249249249248ULL,
	0x1249249249249249ULL,
	0x3249249249249249ULL,
	0x3649249249249249ULL,
	0x36C9249249249249ULL,
	0x36D9249249249249ULL,
	0x36DB249249249249ULL,
	0x36DB649249249249ULL,
	0x36DB6C9249249249ULL,
	0x36DB6D9249249249ULL,
	0x36DB6DB249249249ULL,
	0x36DB6DB649249249ULL,
	0x36DB6DB6C9249249ULL,
	0x36DB6DB6D9249249ULL,
	0x36DB6DB6DB249249ULL,
	0x36DB6DB6DB649249ULL,
	0x36DB6DB6DB6C9249ULL,
	0x36DB6DB6DB6D9249ULL,
	0x36DB6DB6DB6DB249ULL,
	0x36DB6DB6DB6DB649ULL,
	0x36DB6DB6DB6DB6C9ULL,
	0x36DB6DB6DB6DB6D9ULL,
	0x36DB6DB6DB6DB6DBULL,
	0x76DB6DB6DB6DB6DBULL,
	0x7EDB6DB6DB6DB6DBULL,
	0x7FDB6DB6DB6DB6DBULL,
	0x7FFB6DB6DB6DB6DBULL,
	0x7FFF6DB6DB6DB6DBULL,
	0x7FFFEDB6DB6DB6DBULL,
	0x7FFFFDB6DB6DB6DBULL,
	0x7FFFFFB6DB6DB6DBULL,
	0x7FFFFFF6DB6DB6DBULL,
	0x7FFFFFFEDB6DB6DBULL,
	0x7FFFFFFFDB6DB6DBULL,
	0x7FFFFFFFFB6DB6DBULL,
	0x7FFFFFFFFF6DB6DBULL,
	0x7FFFFFFFFFEDB6DBULL,
	0x7FFFFFFFFFFDB6DBULL,
	0x7FFFFFFFFFFFB6DBULL,
	0x7FFFFFFFFFFFF6DBULL,
	0x7FFFFFFFFFFFFEDBULL,
	0x7FFFFFFFFFFFFFDBULL,
	0x7FFFFFFFFFFFFFFBULL,
};

unsigned int getRankWT2_512_counter40(unsigned long long code, unsigned int codeLen, unsigned int i, WT *wt, unsigned int wtLevel) {
	int nextNode = (code >> wtLevel) & 1;

	unsigned int j = i / 448;
	unsigned long long *p = wt->alignedBits + 8 * j;
	unsigned int rank = (*p) & 0x00000000FFFFFFFFULL;
	unsigned int b1 = ((*p) >> 56) & 0x00000000000000FFULL;  // popcount for 128-bit prefix
	unsigned int b2 = b1 + (((*p) >> 48) & 0x00000000000000FFULL);  // popcount for 256-bit prefix
	unsigned int b3 = b2 + (((*p) >> 40) & 0x00000000000000FFULL);  // popcount for 384-bit prefix
	unsigned int temp1, temp2;
	++p;

	unsigned int k = i - (j * 448);
	switch (k / 64) {
	case 6:
		rank += b3 + __builtin_popcountll(*(p + 6) & bitMask[k % 64]);
		break;
	case 5:
		temp1 = __builtin_popcountll(*(p + 4));
		temp2 = __builtin_popcountll(*(p + 5) & bitMask[k % 64]);
		rank += b2 + temp1 + temp2;
		break;
	case 4:
		rank += b2 + __builtin_popcountll(*(p + 4) & bitMask[k % 64]);
		break;
	case 3:
		temp1 = __builtin_popcountll(*(p + 2));
		temp2 = __builtin_popcountll(*(p + 3) & bitMask[k % 64]);
		rank += b1 + temp1 + temp2;
		break;
	case 2:
		rank += b1 + __builtin_popcountll(*(p + 2) & bitMask[k % 64]);
		break;
	case 1:
		temp1 = __builtin_popcountll(*p);
		temp2 = __builtin_popcountll(*(p + 1) & bitMask[k % 64]);
		rank += temp1 + temp2;
		break;
	case 0:
		rank += __builtin_popcountll(*p & bitMask[k % 64]);
	}
	if (nextNode == 0) rank = i - rank;

	if (codeLen == (wtLevel + 1)) return rank;
	__builtin_prefetch(wt->nodes[nextNode]->alignedBits + 8 * (rank / 448), 0, 3);
	return getRankWT2_512_counter40(code, codeLen, rank, wt->nodes[nextNode], wtLevel + 1);
}

unsigned int count_WT2_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen) {
	unsigned char c;
	__builtin_prefetch(wt->alignedBits + 8 * ((firstVal - 1) / 448), 0, 3);
	__builtin_prefetch(wt->alignedBits + 8 * (lastVal / 448), 0, 3);

	while (firstVal <= lastVal && i > 1)
	{
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT2_512_counter40(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		__builtin_prefetch(wt->alignedBits + 8 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRankWT2_512_counter40(code[c], codeLen[c], lastVal, wt, 0);
		__builtin_prefetch(wt->alignedBits + 8 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT2_512_counter40(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		lastVal = C[c] + getRankWT2_512_counter40(code[c], codeLen[c], lastVal, wt, 0);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;

}

unsigned int getRankWT2_1024_counter32(unsigned long long code, unsigned int codeLen, unsigned int i, WT *wt, unsigned int wtLevel) {
	int nextNode = (code >> wtLevel) & 1;

	unsigned int j = i / 960;
	unsigned long long *p = wt->alignedBits + 16 * j;
	unsigned int rank = (*p) & 0x00000000FFFFFFFFULL;
	unsigned int b1 = ((*p) >> 56) & 0x00000000000000FFULL;  // popcount for 192-bit prefix
	unsigned int b2 = b1 + (((*p) >> 48) & 0x00000000000000FFULL);  // popcount for 384-bit prefix
	unsigned int b3 = b2 + (((*p) >> 40) & 0x00000000000000FFULL);  // popcount for 576-bit prefix
	unsigned int b4 = b3 + (((*p) >> 32) & 0x00000000000000FFULL);  // popcount for 768-bit prefix
	unsigned int temp1, temp2, temp3;
	++p;

	unsigned int k = i - (j * 960);
	switch (k / 64) {
	case 14:
		temp1 = __builtin_popcountll(*(p + 12));
		temp2 = __builtin_popcountll(*(p + 13));
		temp3 = __builtin_popcountll(*(p + 14) & bitMask[k % 64]);
		rank += b4 + temp1 + temp2 + temp3;
		break;
	case 13:
		temp1 = __builtin_popcountll(*(p + 12));
		temp2 = __builtin_popcountll(*(p + 13) & bitMask[k % 64]);
		rank += b4 + temp1 + temp2;
		break;
	case 12:
		rank += b4 + __builtin_popcountll(*(p + 12) & bitMask[k % 64]);
		break;
	case 11:
		temp1 = __builtin_popcountll(*(p + 9));
		temp2 = __builtin_popcountll(*(p + 10));
		temp3 = __builtin_popcountll(*(p + 11) & bitMask[k % 64]);
		rank += b3 + temp1 + temp2 + temp3;
		break;
	case 10:
		temp1 = __builtin_popcountll(*(p + 9));
		temp2 = __builtin_popcountll(*(p + 10) & bitMask[k % 64]);
		rank += b3 + temp1 + temp2;
		break;
	case 9:
		rank += b3 + __builtin_popcountll(*(p + 9) & bitMask[k % 64]);
		break;
	case 8:
		temp1 = __builtin_popcountll(*(p + 6));
		temp2 = __builtin_popcountll(*(p + 7));
		temp3 = __builtin_popcountll(*(p + 8) & bitMask[k % 64]);
		rank += b2 + temp1 + temp2 + temp3;
		break;
	case 7:
		temp1 = __builtin_popcountll(*(p + 6));
		temp2 = __builtin_popcountll(*(p + 7) & bitMask[k % 64]);
		rank += b2 + temp1 + temp2;
		break;
	case 6:
		rank += b2 + __builtin_popcountll(*(p + 6) & bitMask[k % 64]);
		break;
	case 5:
		temp1 = __builtin_popcountll(*(p + 3));
		temp2 = __builtin_popcountll(*(p + 4));
		temp3 = __builtin_popcountll(*(p + 5) & bitMask[k % 64]);
		rank += b1 + temp1 + temp2 + temp3;
		break;
	case 4:
		temp1 = __builtin_popcountll(*(p + 3));
		temp2 = __builtin_popcountll(*(p + 4) & bitMask[k % 64]);
		rank += b1 + temp1 + temp2;
		break;
	case 3:
		rank += b1 + __builtin_popcountll(*(p + 3) & bitMask[k % 64]);
		break;
	case 2:
		temp1 = __builtin_popcountll(*p);
		temp2 = __builtin_popcountll(*(p + 1));
		temp3 = __builtin_popcountll(*(p + 2) & bitMask[k % 64]);
		rank += temp1 + temp2 + temp3;
		break;
	case 1:
		temp1 = __builtin_popcountll(*p);
		temp2 = __builtin_popcountll(*(p + 1) & bitMask[k % 64]);
		rank += temp1 + temp2;
		break;
	case 0:
		rank += __builtin_popcountll(*p & bitMask[k % 64]);
	}

	if (nextNode == 0) rank = i - rank;

	if (codeLen == (wtLevel + 1)) return rank;
	__builtin_prefetch(wt->nodes[nextNode]->alignedBits + 16 * (rank / 960), 0, 3);
	return getRankWT2_1024_counter32(code, codeLen, rank, wt->nodes[nextNode], wtLevel + 1);
}

unsigned int count_WT2_1024_counter32(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen) {
	unsigned char c;
	__builtin_prefetch(wt->alignedBits + 16 * ((firstVal - 1) / 960), 0, 3);
	__builtin_prefetch(wt->alignedBits + 16 * (lastVal / 960), 0, 3);

	while (firstVal <= lastVal && i > 1)
	{
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT2_1024_counter32(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		__builtin_prefetch(wt->alignedBits + 16 * ((firstVal - 1) / 960), 0, 3);
		lastVal = C[c] + getRankWT2_1024_counter32(code[c], codeLen[c], lastVal, wt, 0);
		__builtin_prefetch(wt->alignedBits + 16 * (lastVal / 960), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT2_1024_counter32(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		lastVal = C[c] + getRankWT2_1024_counter32(code[c], codeLen[c], lastVal, wt, 0);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;

}

unsigned long long getWT4BitVector0R(unsigned long long b) {
	b = ~b;
	return (b & (b >> 1)) & 0x5555555555555555ULL;
}

unsigned long long getWT4BitVector0L(unsigned long long b) {
	b = ~b;
	return (b & (b << 1)) & 0xAAAAAAAAAAAAAAAAULL;
}

unsigned long long getWT4BitVector1R(unsigned long long b) {
	b ^= 0xAAAAAAAAAAAAAAAAULL;
	return (b & (b >> 1)) & 0x5555555555555555ULL;
}

unsigned long long getWT4BitVector1L(unsigned long long b) {
	b ^= 0xAAAAAAAAAAAAAAAAULL;
	return (b & (b << 1)) & 0xAAAAAAAAAAAAAAAAULL;
}

unsigned long long getWT4BitVector2R(unsigned long long b) {
	b ^= 0x5555555555555555ULL;
	return (b & (b >> 1)) & 0x5555555555555555ULL;
}

unsigned long long getWT4BitVector2L(unsigned long long b) {
	b ^= 0x5555555555555555ULL;
	return (b & (b << 1)) & 0xAAAAAAAAAAAAAAAAULL;
}

unsigned long long getWT4BitVector3R(unsigned long long b) {
	return (b & (b >> 1)) & 0x5555555555555555ULL;
}

unsigned long long getWT4BitVector3L(unsigned long long b) {
	return (b & (b << 1)) & 0xAAAAAAAAAAAAAAAAULL;
}

unsigned int getRankWT4_512(unsigned long long code, unsigned int codeLen, unsigned int i, WT *wt, unsigned int wtLevel) {
	int nextNode = (code >> (2 * wtLevel)) & 3;

	unsigned int rank;
	unsigned int j = i / 192;
	unsigned long long *p = wt->alignedBits + 8 * j + nextNode / 2;
	if ((nextNode & 1) == 0) rank = (*p) >> 32;
	else rank = (*p) & 0x00000000FFFFFFFFULL;
	p += (2 - nextNode / 2);

	i -= (j * 192);
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	unsigned int temp3 = 0;
	switch (nextNode) {
	case 0:
		switch (i / 64) {
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1))) & interlacedMask2[i % 64]);
		}
		break;
	case 1:
		switch (i / 64) {
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1))) & interlacedMask2[i % 64]);
		}
		break;
	case 2:
		switch (i / 64) {
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1))) & interlacedMask2[i % 64]);
		}
		break;
	default:
		switch (i / 64) {
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1))) & interlacedMask2[i % 64]);
		}
	}
	rank += temp1 + temp2 + temp3;

	if (codeLen == (wtLevel + 1)) return rank;
	__builtin_prefetch(wt->nodes[nextNode]->alignedBits + 8 * (rank / 192), 0, 3);
	return getRankWT4_512(code, codeLen, rank, wt->nodes[nextNode], wtLevel + 1);
}

unsigned int count_WT4_512(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen) {
	unsigned char c;
	__builtin_prefetch(wt->alignedBits + 8 * ((firstVal - 1) / 192), 0, 3);
	__builtin_prefetch(wt->alignedBits + 8 * (lastVal / 192), 0, 3);

	while (firstVal <= lastVal && i > 1)
	{
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT4_512(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		__builtin_prefetch(wt->alignedBits + 8 * ((firstVal - 1) / 192), 0, 3);
		lastVal = C[c] + getRankWT4_512(code[c], codeLen[c], lastVal, wt, 0);
		__builtin_prefetch(wt->alignedBits + 8 * (lastVal / 192), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT4_512(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		lastVal = C[c] + getRankWT4_512(code[c], codeLen[c], lastVal, wt, 0);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;

}

unsigned int getRankWT4_1024(unsigned long long code, unsigned int codeLen, unsigned int i, WT *wt, unsigned int wtLevel) {
	int nextNode = (code >> (2 * wtLevel)) & 3;

	unsigned int rank;
	unsigned int j = i / 448;
	unsigned long long *p = wt->alignedBits + 16 * j + nextNode / 2;
	if ((nextNode & 1) == 0) rank = (*p) >> 32;
	else rank = (*p) & 0x00000000FFFFFFFFULL;
	p += (2 - nextNode / 2);

	i -= (j * 448);
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	unsigned int temp3 = 0;
	unsigned int temp4 = 0;
	unsigned int temp5 = 0;
	unsigned int temp6 = 0;
	unsigned int temp7 = 0;
	switch (nextNode) {
	case 0:
		switch (i / 64) {
		case 6:
			temp7 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 5:
			temp6 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 4:
			temp5 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 3:
			temp4 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector0R(*p) | getWT4BitVector0L(*(p + 1))) & interlacedMask2[i % 64]);
		}
		break;
	case 1:
		switch (i / 64) {
		case 6:
			temp7 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 5:
			temp6 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 4:
			temp5 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 3:
			temp4 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector1R(*p) | getWT4BitVector1L(*(p + 1))) & interlacedMask2[i % 64]);
		}
		break;
	case 2:
		switch (i / 64) {
		case 6:
			temp7 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 5:
			temp6 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 4:
			temp5 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 3:
			temp4 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector2R(*p) | getWT4BitVector2L(*(p + 1))) & interlacedMask2[i % 64]);
		}
		break;
	default:
		switch (i / 64) {
		case 6:
			temp7 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 5:
			temp6 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 4:
			temp5 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 3:
			temp4 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 2:
			temp3 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 1:
			temp2 = __builtin_popcountll(getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT4BitVector3R(*p) | getWT4BitVector3L(*(p + 1))) & interlacedMask2[i % 64]);
		}
	}
	rank += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7;

	if (codeLen == (wtLevel + 1)) return rank;
	__builtin_prefetch(wt->nodes[nextNode]->alignedBits + 16 * (rank / 448), 0, 3);
	return getRankWT4_1024(code, codeLen, rank, wt->nodes[nextNode], wtLevel + 1);
}

unsigned int count_WT4_1024(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen) {
	unsigned char c;
	__builtin_prefetch(wt->alignedBits + 16 * ((firstVal - 1) / 448), 0, 3);
	__builtin_prefetch(wt->alignedBits + 16 * (lastVal / 448), 0, 3);

	while (firstVal <= lastVal && i > 1)
	{
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT4_1024(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		__builtin_prefetch(wt->alignedBits + 16 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRankWT4_1024(code[c], codeLen[c], lastVal, wt, 0);
		__builtin_prefetch(wt->alignedBits + 16 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT4_1024(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		lastVal = C[c] + getRankWT4_1024(code[c], codeLen[c], lastVal, wt, 0);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;

}

unsigned long long getWT8BitVector0R(unsigned long long b) {
	b = ~b;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector1R(unsigned long long b) {
	b ^= 0x6DB6DB6DB6DB6DB6ULL;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector2R(unsigned long long b) {
	b ^= 0x5B6DB6DB6DB6DB6DULL;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector3R(unsigned long long b) {
	b ^= 0x4924924924924924ULL;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector4R(unsigned long long b) {
	b ^= 0x36DB6DB6DB6DB6DBULL;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector5R(unsigned long long b) {
	b ^= 0x2492492492492492ULL;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector6R(unsigned long long b) {
	b ^= 0x1249249249249249ULL;
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector7R(unsigned long long b) {
	return (b & (b >> 1) & (b >> 2)) & 0x1249249249249249ULL;
}

unsigned long long getWT8BitVector0M(unsigned long long b) {
	b = ~b;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector1M(unsigned long long b) {
	b ^= 0x6DB6DB6DB6DB6DB6ULL;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector2M(unsigned long long b) {
	b ^= 0x5B6DB6DB6DB6DB6DULL;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector3M(unsigned long long b) {
	b ^= 0x4924924924924924ULL;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector4M(unsigned long long b) {
	b ^= 0x36DB6DB6DB6DB6DBULL;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector5M(unsigned long long b) {
	b ^= 0x2492492492492492ULL;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector6M(unsigned long long b) {
	b ^= 0x1249249249249249ULL;
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector7M(unsigned long long b) {
	return (b & (b >> 1) & (b << 1)) & 0x2492492492492492ULL;
}

unsigned long long getWT8BitVector0L(unsigned long long b) {
	b = ~b;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector1L(unsigned long long b) {
	b ^= 0x6DB6DB6DB6DB6DB6ULL;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector2L(unsigned long long b) {
	b ^= 0x5B6DB6DB6DB6DB6DULL;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector3L(unsigned long long b) {
	b ^= 0x4924924924924924ULL;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector4L(unsigned long long b) {
	b ^= 0x36DB6DB6DB6DB6DBULL;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector5L(unsigned long long b) {
	b ^= 0x2492492492492492ULL;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector6L(unsigned long long b) {
	b ^= 0x1249249249249249ULL;
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned long long getWT8BitVector7L(unsigned long long b) {
	return (b & (b << 1) & (b << 2)) & 0x4924924924924924ULL;
}

unsigned int getRankWT8_512(unsigned long long code, unsigned int codeLen, unsigned int i, WT *wt, unsigned int wtLevel) {
	int nextNode = (code >> (3 * wtLevel)) & 7;

	unsigned int rank;
	unsigned int j = i / 84;
	unsigned long long *p = wt->alignedBits + 8 * j + nextNode / 2;
	if ((nextNode & 1) == 0) rank = (*p) >> 32;
	else rank = (*p) & 0x00000000FFFFFFFFULL;
	p += (4 - nextNode / 2);

	i -= (j * 84);
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	switch (nextNode) {
	case 0:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector0R(*p) | getWT8BitVector0M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector0R(*p) | getWT8BitVector0M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	case 1:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector1R(*p) | getWT8BitVector1M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector1R(*p) | getWT8BitVector1M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	case 2:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector2R(*p) | getWT8BitVector2M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector2R(*p) | getWT8BitVector2M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	case 3:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector3R(*p) | getWT8BitVector3M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector3R(*p) | getWT8BitVector3M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	case 4:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector4R(*p) | getWT8BitVector4M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector4R(*p) | getWT8BitVector4M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	case 5:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector5R(*p) | getWT8BitVector5M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector5R(*p) | getWT8BitVector5M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	case 6:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector6R(*p) | getWT8BitVector6M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector6R(*p) | getWT8BitVector6M(*(p + 1))) & interlacedMask3[i % 42]);
		}
		break;
	default:
		switch (i / 42) {
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector7R(*p) | getWT8BitVector7M(*(p + 1)));
			p += 2;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector7R(*p) | getWT8BitVector7M(*(p + 1))) & interlacedMask3[i % 42]);
		}
	}
	rank += temp1 + temp2;

	if (codeLen == (wtLevel + 1)) return rank;
	__builtin_prefetch(wt->nodes[nextNode]->alignedBits + 8 * (rank / 84), 0, 3);
	return getRankWT8_512(code, codeLen, rank, wt->nodes[nextNode], wtLevel + 1);
}

unsigned int count_WT8_512(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen) {
	unsigned char c;
	__builtin_prefetch(wt->alignedBits + 8 * ((firstVal - 1) / 84), 0, 3);
	__builtin_prefetch(wt->alignedBits + 8 * (lastVal / 84), 0, 3);

	while (firstVal <= lastVal && i > 1)
	{
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT8_512(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		__builtin_prefetch(wt->alignedBits + 8 * ((firstVal - 1) / 84), 0, 3);
		lastVal = C[c] + getRankWT8_512(code[c], codeLen[c], lastVal, wt, 0);
		__builtin_prefetch(wt->alignedBits + 8 * (lastVal / 84), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT8_512(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		lastVal = C[c] + getRankWT8_512(code[c], codeLen[c], lastVal, wt, 0);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;

}

unsigned int getRankWT8_1024(unsigned long long code, unsigned int codeLen, unsigned int i, WT *wt, unsigned int wtLevel) {
	int nextNode = (code >> (3 * wtLevel)) & 7;

	unsigned int rank;
	unsigned int j = i / 252;
	unsigned long long *p = wt->alignedBits + 16 * j + nextNode / 2;
	if ((nextNode & 1) == 0) rank = (*p) >> 32;
	else rank = (*p) & 0x00000000FFFFFFFFULL;
	p += (4 - nextNode / 2);

	i -= (j * 252);
	unsigned int temp1 = 0;
	unsigned int temp2 = 0;
	unsigned int temp3 = 0;
	unsigned int temp4 = 0;
	switch (nextNode) {
	case 0:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector0R(*p) | getWT8BitVector0M(*(p + 1)) | getWT8BitVector0L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector0R(*p) | getWT8BitVector0M(*(p + 1)) | getWT8BitVector0L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector0R(*p) | getWT8BitVector0M(*(p + 1)) | getWT8BitVector0L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector0R(*p) | getWT8BitVector0M(*(p + 1)) | getWT8BitVector0L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	case 1:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector1R(*p) | getWT8BitVector1M(*(p + 1)) | getWT8BitVector1L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector1R(*p) | getWT8BitVector1M(*(p + 1)) | getWT8BitVector1L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector1R(*p) | getWT8BitVector1M(*(p + 1)) | getWT8BitVector1L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector1R(*p) | getWT8BitVector1M(*(p + 1)) | getWT8BitVector1L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	case 2:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector2R(*p) | getWT8BitVector2M(*(p + 1)) | getWT8BitVector2L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector2R(*p) | getWT8BitVector2M(*(p + 1)) | getWT8BitVector2L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector2R(*p) | getWT8BitVector2M(*(p + 1)) | getWT8BitVector2L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector2R(*p) | getWT8BitVector2M(*(p + 1)) | getWT8BitVector2L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	case 3:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector3R(*p) | getWT8BitVector3M(*(p + 1)) | getWT8BitVector3L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector3R(*p) | getWT8BitVector3M(*(p + 1)) | getWT8BitVector3L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector3R(*p) | getWT8BitVector3M(*(p + 1)) | getWT8BitVector3L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector3R(*p) | getWT8BitVector3M(*(p + 1)) | getWT8BitVector3L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	case 4:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector4R(*p) | getWT8BitVector4M(*(p + 1)) | getWT8BitVector4L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector4R(*p) | getWT8BitVector4M(*(p + 1)) | getWT8BitVector4L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector4R(*p) | getWT8BitVector4M(*(p + 1)) | getWT8BitVector4L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector4R(*p) | getWT8BitVector4M(*(p + 1)) | getWT8BitVector4L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	case 5:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector5R(*p) | getWT8BitVector5M(*(p + 1)) | getWT8BitVector5L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector5R(*p) | getWT8BitVector5M(*(p + 1)) | getWT8BitVector5L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector5R(*p) | getWT8BitVector5M(*(p + 1)) | getWT8BitVector5L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector5R(*p) | getWT8BitVector5M(*(p + 1)) | getWT8BitVector5L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	case 6:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector6R(*p) | getWT8BitVector6M(*(p + 1)) | getWT8BitVector6L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector6R(*p) | getWT8BitVector6M(*(p + 1)) | getWT8BitVector6L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector6R(*p) | getWT8BitVector6M(*(p + 1)) | getWT8BitVector6L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector6R(*p) | getWT8BitVector6M(*(p + 1)) | getWT8BitVector6L(*(p + 2))) & interlacedMask3[i % 63]);
		}
		break;
	default:
		switch (i / 63) {
		case 3:
			temp4 = __builtin_popcountll(getWT8BitVector7R(*p) | getWT8BitVector7M(*(p + 1)) | getWT8BitVector7L(*(p + 2)));
			p += 3;
		case 2:
			temp3 = __builtin_popcountll(getWT8BitVector7R(*p) | getWT8BitVector7M(*(p + 1)) | getWT8BitVector7L(*(p + 2)));
			p += 3;
		case 1:
			temp2 = __builtin_popcountll(getWT8BitVector7R(*p) | getWT8BitVector7M(*(p + 1)) | getWT8BitVector7L(*(p + 2)));
			p += 3;
		case 0:
			temp1 = __builtin_popcountll((getWT8BitVector7R(*p) | getWT8BitVector7M(*(p + 1)) | getWT8BitVector7L(*(p + 2))) & interlacedMask3[i % 63]);
		}
	}
	rank += temp1 + temp2 + temp3 + temp4;

	if (codeLen == (wtLevel + 1)) return rank;
	__builtin_prefetch(wt->nodes[nextNode]->alignedBits + 16 * (rank / 252), 0, 3);
	return getRankWT8_1024(code, codeLen, rank, wt->nodes[nextNode], wtLevel + 1);
}

unsigned int count_WT8_1024(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen) {
	unsigned char c;
	__builtin_prefetch(wt->alignedBits + 16 * ((firstVal - 1) / 252), 0, 3);
	__builtin_prefetch(wt->alignedBits + 16 * (lastVal / 252), 0, 3);

	while (firstVal <= lastVal && i > 1)
	{
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT8_1024(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		__builtin_prefetch(wt->alignedBits + 16 * ((firstVal - 1) / 252), 0, 3);
		lastVal = C[c] + getRankWT8_1024(code[c], codeLen[c], lastVal, wt, 0);
		__builtin_prefetch(wt->alignedBits + 16 * (lastVal / 252), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = pattern[i - 1];
		firstVal = C[c] + getRankWT8_1024(code[c], codeLen[c], firstVal - 1, wt, 0) + 1;
		lastVal = C[c] + getRankWT8_1024(code[c], codeLen[c], lastVal, wt, 0);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;

}
