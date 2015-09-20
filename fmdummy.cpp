#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <string>
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
		this->ordChars = breakByDelimeter(selectedChars, ',', this->ordCharsLen);
	}
}

void FMDummy1::setFunctions() {
	switch (this->type) {
	case FMDummy1::TYPE_512c:
		this->builder = &buildRank_64_512_counter40;
		this->countOperation = &count_64_512_counter40;
		break;
	default:
		this->builder = &buildRank_64_256_counter48;
		this->countOperation = &count_64_256_counter48;
	}
}

void FMDummy1::free() {
	this->freeMemory();
	this->initialize();
}

void FMDummy1::initialize() {
	for (int i = 0; i < 256; ++i) {
		this->raw_bwtWithRanks[i] = NULL;
	}
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
	for (unsigned int i = 0; i < 256; ++i) {
		if (this->raw_bwtWithRanks[i] != NULL) delete[] this->raw_bwtWithRanks[i];
	}
	if (this->bwtWithRanks != NULL) delete[] this->bwtWithRanks;
	if (this->ordChars != NULL) delete[] this->ordChars;
}

void FMDummy1::build(unsigned char* text, unsigned int textLen) {
	checkNullChar(text, textLen);
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
		this->textSize = textLen;
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
		for (long long i = 0; i < bwtDenseInLongLen; ++i) {
			bwtDenseInLong[ordChar][i] = ((unsigned long long)bwtDense[8 * i + 7] << 56) | ((unsigned long long)bwtDense[8 * i + 6] << 48) | ((unsigned long long)bwtDense[8 * i + 5] << 40) | ((unsigned long long)bwtDense[8 * i + 4] << 32) | ((unsigned long long)bwtDense[8 * i + 3] << 24) | ((unsigned long long)bwtDense[8 * i + 2] << 16) | ((unsigned long long)bwtDense[8 * i + 1] << 8) | (unsigned long long)bwtDense[8 * i];
		}
		for (long long i = bwtDenseInLongLen; i < bwtDenseInLongLen + 8; ++i) {
			bwtDenseInLong[ordChar][i] = 0ULL;
		}
		delete[] bwtDense;
	}
	delete[] bwt;
	if (this->verbose) cout << "Done" << endl;

	this->c = getArrayC(text, textLen, verbose);

	if (this->verbose) cout << "Interweaving BWT with ranks ... " << flush;
	this->bwtWithRanks = builder(bwtDenseInLong, bwtDenseInLongLen, this->raw_bwtWithRanks, this->ordChars, this->ordCharsLen, this->raw_bwtWithRanksLen);
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
	return this->countOperation(pattern, patternLen, this->c, this->bwtWithRanks);
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
	this->bwtWithRanks = new unsigned long long*[256];
	for (unsigned int i = 0; i < this->ordCharsLen; ++i) {
		unsigned int c = this->ordChars[i];
		this->raw_bwtWithRanks[c] = new unsigned long long[this->raw_bwtWithRanksLen];
		fread(this->raw_bwtWithRanks[c], (size_t)sizeof(unsigned long long), (size_t)this->raw_bwtWithRanksLen, inFile);
		unsigned long long *res = raw_bwtWithRanks[c];
		while ((unsigned long long)res % 128) {
			++res;
		}
		this->bwtWithRanks[c] = res;
	}
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

unsigned long long** buildRank_64_256(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
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

unsigned long long** buildRank_64_256_counter48(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
	unsigned long long *p, pops, rank, b1, b2;
	unsigned long long **result = new unsigned long long*[256];
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

unsigned long long** buildRank_64_512(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
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

unsigned long long** buildRank_64_512_counter40(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen) {
	unsigned long long *p, pop1, pop2, pop3, rank, b1, b2, b3;
	unsigned long long **result = new unsigned long long*[256];
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

unsigned int getRank_64_256(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
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

unsigned int count_64_256(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks){
	int i = m - 1;
	unsigned char c = Q[i];
	unsigned int firstVal = C[c] + 1;
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 4 * ((firstVal - 1) / 192), 0, 3);
	unsigned int lastVal = C[c + 1];
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 4 * (lastVal / 192), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_256(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 4 * ((firstVal - 1) / 192), 0, 3);
		lastVal = C[c] + getRank_64_256(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 4 * (lastVal / 192), 0, 3);
		--i;
	}

	if (firstVal <= lastVal)
	{
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_256(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_64_256(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int getRank_64_256_counter48(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
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

unsigned int count_64_256_counter48(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks){
	int i = m - 1;
	unsigned char c = Q[i];
	unsigned int firstVal = C[c] + 1;
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 4 * ((firstVal - 1) / 192), 0, 3);
	unsigned int lastVal = C[c + 1];
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 4 * (lastVal / 192), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_256_counter48(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 4 * ((firstVal - 1) / 192), 0, 3);
		lastVal = C[c] + getRank_64_256_counter48(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 4 * (lastVal / 192), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_256_counter48(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_64_256_counter48(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int getRank_64_512(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {
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

unsigned int count_64_512(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks){
	int i = m - 1;
	unsigned char c = Q[i];
	unsigned int firstVal = C[c] + 1;
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 8 * ((firstVal - 1) / 448), 0, 3);
	unsigned int lastVal = C[c + 1];
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 8 * (lastVal / 448), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_512(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 8 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRank_64_512(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 8 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_512(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_64_512(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}

unsigned int getRank_64_512_counter40(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks) {

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

unsigned int count_64_512_counter40(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks){
	int i = m - 1;
	unsigned char c = Q[i];
	unsigned int firstVal = C[c] + 1;
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 8 * ((firstVal - 1) / 448), 0, 3);
	unsigned int lastVal = C[c + 1];
	__builtin_prefetch(bwtWithRanks[Q[i - 1]] + 8 * (lastVal / 448), 0, 3);

	while (firstVal <= lastVal && i > 1) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_512_counter40(c, firstVal - 1, bwtWithRanks) + 1;
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 8 * ((firstVal - 1) / 448), 0, 3);
		lastVal = C[c] + getRank_64_512_counter40(c, lastVal, bwtWithRanks);
		__builtin_prefetch(bwtWithRanks[Q[i - 2]] + 8 * (lastVal / 448), 0, 3);
		--i;
	}

	if (firstVal <= lastVal) {
		c = Q[i - 1];
		firstVal = C[c] + getRank_64_512_counter40(c, firstVal - 1, bwtWithRanks) + 1;
		lastVal = C[c] + getRank_64_512_counter40(c, lastVal, bwtWithRanks);
	}

	if (firstVal > lastVal) return 0;
	else return lastVal - firstVal + 1;
}
