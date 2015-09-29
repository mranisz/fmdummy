#include "shared/api.h"
#include <cstdio>
#include <string>
#include <vector>

using namespace std;

#ifndef FMDUMMY_H_
#define FMDUMMY_H_

/*FMDUMMY1*/

class FMDummy1 : public I_Api {
private:
	unsigned long long *bwtWithRanks[256];
	unsigned int bwtWithRanksLen;
	unsigned long long **alignedBWTWithRanks;
	unsigned int *ordChars;
	unsigned int ordCharsLen;
	unsigned int c[257];

	int type;
	bool allChars;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned int *, unsigned int, unsigned long long **, unsigned int &);
	unsigned int (*countOperation)(unsigned char*, unsigned int, unsigned int *, unsigned long long **, unsigned int, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType);
	void setFunctions();
	void setSelectedChars(string selectedChars);

public:
	enum IndexTypesConst {
		TYPE_256 = 1,
		TYPE_512 = 2
	};

	FMDummy1() {
		this->initialize();
	}

	FMDummy1(string indexType, string selectedChars) {
		this->initialize();
		this->setType(indexType);
		this->setSelectedChars(selectedChars);
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

class FMDummy2 : public I_Api {
private:
	unsigned long long *bwtWithRanks[256];
	unsigned int bwtWithRanksLen;
	unsigned long long **alignedBWTWithRanks;
	unsigned char *encodedChars[256];
	unsigned int encodedCharsLen[256];
	unsigned int c[257];
	unsigned int bInC;

	int type;
	int schema;
	int bitsPerChar;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned int *, unsigned int, unsigned long long **, unsigned int &);
	unsigned int (*countOperation)(unsigned char*, unsigned int, unsigned int *, unsigned long long **, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType, string schema);
	void setBitsPerChar(string bitsPerChar);
	void setFunctions();

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
	}

	FMDummy2(string indexType, string schema, string bitsPerChar) {
		this->initialize();
		this->setType(indexType, schema);
		this->setBitsPerChar(bitsPerChar);
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

class FMDummy3 : public I_Api {
private:
	unsigned char *bwtWithRanks;
	unsigned int bwtWithRanksLen;
	unsigned char *alignedBWTWithRanks;
	unsigned int lut[256][125];
	unsigned int c[257];

	int type;

	unsigned int textSize;

	unsigned int (*countOperation)(unsigned char *, unsigned int, unsigned int *, unsigned char *, unsigned int lut[][125], unsigned int, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType);
	void setFunctions();
	void buildRank_512_enc125(unsigned char *bwtEnc125, unsigned int bwtLen);
	void buildRank_1024_enc125(unsigned char *bwtEnc125, unsigned int bwtLen);

public:
	enum IndexTypesConst {
		TYPE_512 = 1,
		TYPE_1024 = 2
	};

	FMDummy3() {
		this->initialize();
	}

	FMDummy3(string indexType) {
		this->initialize();
		this->setType(indexType);
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

/*WT*/

class WT {
private:
	void freeMemory();

public:
	unsigned long long *bits;
	unsigned int bitsLen;
	unsigned long long *alignedBits;
	WT** nodes;
	unsigned int nodesLen;

	WT() {}

	WT(int wtType) {
		this->nodesLen = wtType;
		this->nodes = new WT *[this->nodesLen];
	};

	unsigned int getWTSize();
	void save(FILE *outFile);
	void load(FILE *inFile);

	~WT() {
		this->freeMemory();
	}
};

/*FMDUMMYWT*/

class FMDummyWT : public I_Api {
private:
	WT *wt;
	unsigned long long code[256];
	unsigned int codeLen[256];
	unsigned int c[257];

	int type;
	int wtType;

	unsigned int textSize;

	unsigned int (*countOperation)(unsigned char *, unsigned int, unsigned int *, WT *, unsigned int, unsigned int, unsigned long long *, unsigned int *);

	void freeMemory();
	void initialize();
	void setType(string wtType, string indexType);
	void setFunctions();
	void createWT(unsigned char *text, unsigned int textLen);
	WT *createWT2_512_counter40(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	WT *createWT2_1024_counter32(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	WT *createWT4(unsigned char *text, unsigned int textLen, unsigned int wtLevel);
	WT *createWT8(unsigned char *text, unsigned int textLen, unsigned int wtLevel);

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
	}

	FMDummyWT(string wtType, string indexType) {
		this->initialize();
		this->setType(wtType, indexType);
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
unsigned int count_SCBO_256_counter48(unsigned char *pattern, unsigned int patternLen, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned int count_CB_256_counter48(unsigned char *pattern, unsigned int patternLen, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned int count_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
unsigned int count_SCBO_512_counter40(unsigned char *pattern, unsigned int patternLen, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned int count_CB_512_counter40(unsigned char *pattern, unsigned int patternLen, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned char *getEncodedInSCBO(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen);
unsigned char *getEncodedInCB(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen, unsigned int &b);
unsigned char* encodePattern(unsigned char* pattern, unsigned int patternLen, unsigned char** encodedChars, unsigned int* encodedCharsLen, unsigned int &encodedPatternLen, bool &wrongEncoding);
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
