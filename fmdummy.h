#include "shared/common.h"
#include "shared/wt.h"
#include "shared/hash.h"
#include <cstdio>
#include <string.h>
#include <vector>

using namespace std;

#ifndef FMDUMMY_H_
#define FMDUMMY_H_

/*FMDUMMY1*/

class FMDummy1 : public Index {
private:
	unsigned long long *bwtWithRanks[256];
	unsigned int bwtWithRanksLen;
	unsigned long long **alignedBWTWithRanks;
	unsigned int *ordChars;
	unsigned int ordCharsLen;
	unsigned int c[257];
	HT *ht;

	int type;
	bool allChars;
	string selectedChars;
	unsigned int k;
	double loadFactor;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned int *, unsigned int, unsigned long long **, unsigned int &);
	unsigned int (FMDummy1::*countOperation)(unsigned char *, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType);
	void setSelectedChars(string selectedChars);
	void setK(unsigned int k);
	void setLoadFactor(double loadFactor);
	void setFunctions();
	unsigned int count_std_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_512_counter40(unsigned char *pattern, unsigned int patternLen);

public:
	enum IndexTypesConst {
		TYPE_256 = 1,
		TYPE_512 = 2
	};

	FMDummy1() {
		this->initialize();
		this->setFunctions();
	}

	FMDummy1(string indexType, string selectedChars) {
		this->initialize();
		this->setType(indexType);
		this->setSelectedChars(selectedChars);
		this->setFunctions();
	}

	FMDummy1(string indexType, string selectedChars, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(indexType);
		this->setSelectedChars(selectedChars);
		this->setK(k);
		this->setLoadFactor(loadFactor);
		this->setFunctions();
	}

	~FMDummy1() {
		this->freeMemory();
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(char *fileName);
	void load(char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

/*FMDUMMY2*/

class FMDummy2 : public Index {
private:
	unsigned long long *bwtWithRanks[256];
	unsigned int bwtWithRanksLen;
	unsigned long long **alignedBWTWithRanks;
	unsigned char *encodedChars;
	unsigned int encodedCharsLen[256];
	unsigned int maxEncodedCharsLen;
	unsigned int c[257];
	unsigned int bInC;
	unsigned char *encodedPattern;
	unsigned int maxPatternLen;
	HT *ht;

	int type;
	int schema;
	int bitsPerChar;
	unsigned int k;
	double loadFactor;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned int *, unsigned int, unsigned long long **, unsigned int &);
	unsigned int (FMDummy2::*countOperation)(unsigned char *, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType, string schema);
	void setBitsPerChar(string bitsPerChar);
	void setMaxEncodedCharsLen();
	void setEncodedPattern(unsigned int maxPatternLen);
	void setK(unsigned int k);
	void setLoadFactor(double loadFactor);
	void setFunctions();
	void encodePattern(unsigned char *pattern, unsigned int patternLen, unsigned int &encodedPatternLen, bool &wrongEncoding);
	unsigned char *getEncodedInSCBO(unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen);
	unsigned char *getEncodedInCB(unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned int &b);
	unsigned int count_std_CB_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_CB_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_SCBO_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_SCBO_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_CB_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_CB_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_SCBO_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_SCBO_512_counter40(unsigned char *pattern, unsigned int patternLen);

public:
	enum IndexTypesConst {
		TYPE_256 = 1,
		TYPE_512 = 2
	};

	enum BitsPerCharConst {
		BITS_3 = 3,
		BITS_4 = 4
	};

	enum SchemaConst {
		SCHEMA_SCBO = 1,
		SCHEMA_CB = 2
	};

	FMDummy2() {
		this->initialize();
		this->setFunctions();
	}

	FMDummy2(string indexType, string schema, string bitsPerChar) {
		this->initialize();
		this->setType(indexType, schema);
		this->setBitsPerChar(bitsPerChar);
		this->setFunctions();
	}

	FMDummy2(string indexType, string schema, string bitsPerChar, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(indexType, schema);
		this->setBitsPerChar(bitsPerChar);
		this->setK(k);
		this->setLoadFactor(loadFactor);
		this->setFunctions();
	}

	~FMDummy2() {
		this->freeMemory();
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(char *fileName);
	void load(char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

/*FMDUMMY3*/

class FMDummy3 : public Index {
private:
	unsigned char *bwtWithRanks;
	unsigned int bwtWithRanksLen;
	unsigned char *alignedBWTWithRanks;
	unsigned int lut[256][125];
	unsigned int c[257];
	HT *ht;

	int type;
	unsigned int k;
	double loadFactor;

	unsigned int textSize;

	unsigned int (FMDummy3::*countOperation)(unsigned char *, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType);
	void setK(unsigned int k);
	void setLoadFactor(double loadFactor);
	void setFunctions();
	void buildRank_512_enc125(unsigned char *bwtEnc125, unsigned int bwtLen);
	void buildRank_1024_enc125(unsigned char *bwtEnc125, unsigned int bwtLen);
	unsigned int count_std_512_enc125(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_1024_enc125(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_512_enc125(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_1024_enc125(unsigned char *pattern, unsigned int patternLen);

public:
	enum IndexTypesConst {
		TYPE_512 = 1,
		TYPE_1024 = 2
	};

	FMDummy3() {
		this->initialize();
		this->setFunctions();
	}

	FMDummy3(string indexType) {
		this->initialize();
		this->setType(indexType);
		this->setFunctions();
	}

	FMDummy3(string indexType, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(indexType);
		this->setK(k);
		this->setLoadFactor(loadFactor);
		this->setFunctions();
	}

	~FMDummy3() {
		this->freeMemory();
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(char *fileName);
	void load(char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

/*FMDUMMYWT*/

class FMDummyWT : public Index {
private:
	WT *wt;
	unsigned long long code[256];
	unsigned int codeLen[256];
	unsigned int c[257];
	HT *ht;

	int type;
	int wtType;
	unsigned int k;
	double loadFactor;

	unsigned int textSize;

	unsigned int (FMDummyWT::*countOperation)(unsigned char *, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string wtType, string indexType);
	void setK(unsigned int k);
	void setLoadFactor(double loadFactor);
	void setFunctions();
	WT *createWT2_512_counter40(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	WT *createWT2_1024_counter32(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	WT *createWT4(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	WT *createWT8(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	unsigned int count_WT2std_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT2hash_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT2std_1024_counter32(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT2hash_1024_counter32(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT4std_512(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT4hash_512(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT4std_1024(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT4hash_1024(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT8std_512(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT8hash_512(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT8std_1024(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_WT8hash_1024(unsigned char *pattern, unsigned int patternLen);

public:
	enum WTTypesConst {
		TYPE_WT2 = 2,
		TYPE_WT4 = 4,
		TYPE_WT8 = 8
	};

	enum IndexTypesConst {
		TYPE_512 = 8,
		TYPE_1024 = 16
	};

	FMDummyWT() {
		this->initialize();
		this->setFunctions();
	}

	FMDummyWT(string wtType, string indexType) {
		this->initialize();
		this->setType(wtType, indexType);
		this->setFunctions();
	}

	FMDummyWT(string wtType, string indexType, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(wtType, indexType);
		this->setK(k);
		this->setLoadFactor(loadFactor);
		this->setFunctions();
	}

	~FMDummyWT() {
		this->freeMemory();
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(char *fileName);
	void load(char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

/*SHARED STUFF*/

unsigned char *getBinDenseForChar(unsigned char *bwt, unsigned int bwtLen, int ordChar);
unsigned long long** buildRank_256_counter48(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned long long** bwtWithRanks, unsigned int &bwtWithRanksLen);
unsigned long long** buildRank_512_counter40(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned long long** bwtWithRanks, unsigned int &bwtWithRanksLen);
unsigned int count_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
void getCountBoundaries_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal, unsigned int &leftBoundary, unsigned int &rightBoundary);
unsigned int count_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
void getCountBoundaries_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal, unsigned int &leftBoundary, unsigned int &rightBoundary);
unsigned char *encode125(unsigned char* text, unsigned int textLen, unsigned int *selectedOrdChars, unsigned int &encodedTextLen);
void fill125LUT(unsigned int *selectedOrdChars, unsigned int lut[][125]);
unsigned int count_512_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal);
unsigned int count_1024_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal);
unsigned int count_WT2_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen);
unsigned int count_WT2_1024_counter32(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen);
unsigned int count_WT4_512(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen);
unsigned int count_WT4_1024(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen);
unsigned int count_WT8_512(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen);
unsigned int count_WT8_1024(unsigned char *pattern, unsigned int i, unsigned int *C, WT *wt, unsigned int firstVal, unsigned int lastVal, unsigned long long *code, unsigned int *codeLen);
void encodeHuff(unsigned int d, unsigned char *text, unsigned int textLen, unsigned long long *huffCode, unsigned int *huffCodeLen);

#endif /* FMDUMMY_H_ */
