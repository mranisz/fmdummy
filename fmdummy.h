#include "shared/api.h"
#include <cstdio>
#include <string>

using namespace std;

#ifndef FMDUMMY_H_
#define FMDUMMY_H_

//FMDUMMY1

class FMDummy1 : public I_Api {
private:
	unsigned long long **raw_bwtWithRanks;
	unsigned long long **bwtWithRanks;
	unsigned int raw_bwtWithRanksLen;
	unsigned int ordCharsLen;
	unsigned int *ordChars;
	unsigned int *c;

	int type;
	bool allChars;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned int *, unsigned int, unsigned int &);
	unsigned int (*countOperation)(unsigned char*, unsigned int, unsigned int *, unsigned long long **, unsigned int, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType);
	void setFunctions();
	void setSelectedChars(string selectedChars);

public:

	enum IndexTypesConst {
		TYPE_256c = 1,
		TYPE_512c = 2
	};

	FMDummy1() {
		this->initialize();
	};

	FMDummy1(string indexType, string selectedChars) {
		this->initialize();
		this->setType(indexType);
		this->setSelectedChars(selectedChars);
	};
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

//FMDUMMY2

class FMDummy2 : public I_Api {
private:
	unsigned long long **raw_bwtWithRanks;
	unsigned long long **bwtWithRanks;
	unsigned int raw_bwtWithRanksLen;
	unsigned char *encodedChars[256];
	unsigned int encodedCharsLen[256];
	unsigned int *c;
	unsigned int bInC;

	int type;
	int schema;
	int bitsPerChar;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned int *, unsigned int, unsigned int &);
	unsigned int (*countOperation)(unsigned char*, unsigned int, unsigned int *, unsigned long long **, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType, string schema);
	void setBitsPerChar(string bitsPerChar);
	void setFunctions();

public:

	enum IndexTypesConst {
		TYPE_256c = 1,
		TYPE_512c = 2
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
	};

	FMDummy2(string indexType, string schema, string bitsPerChar) {
		this->initialize();
		this->setType(indexType, schema);
		this->setBitsPerChar(bitsPerChar);
	};

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

//FMDUMMY3

class FMDummy3 : public I_Api {
private:
	unsigned char *raw_bwtWithRanks;
	unsigned char *bwtWithRanks;
	unsigned int lut[256][125];

	unsigned int raw_bwtWithRanksLen;

	int type;
	unsigned int *c;

	unsigned int textSize;

	unsigned char *(*builder)(unsigned char *, unsigned int, unsigned int [][125], unsigned int &);
	unsigned int (*countOperation)(unsigned char *, unsigned int, unsigned int *, unsigned char *, unsigned int lut[][125], unsigned int, unsigned int);

	void freeMemory();
	void initialize();
	void setType(string indexType);
	void setFunctions();

public:

	enum IndexTypesConst {
		TYPE_512 = 1,
		TYPE_1024 = 2
	};

	FMDummy3() {
		this->initialize();
	};

	FMDummy3(string indexType) {
		this->initialize();
		this->setType(indexType);
	};

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

/*SHARED FUNCTIONS*/

unsigned char *getBinDenseForChar(unsigned char *bwt, unsigned int bwtLen, int ordChar);
unsigned long long** buildRank_256(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned long long** buildRank_256_counter48(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned long long** buildRank_512(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned long long** buildRank_512_counter40(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned int getRank_256(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_256(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks);
unsigned int getRank_256_counter48(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
unsigned int count_SCBO_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned int count_CB_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned int getRank_512(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_512(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks);
unsigned int getRank_512_counter40(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
unsigned int count_SCBO_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned int count_CB_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long** bwtWithRanks, unsigned int bInC);
unsigned char *getEncodedInSCBO(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen);
unsigned char *getEncodedInCB(int bits, unsigned char *text, unsigned int textLen, unsigned int &encodedTextLen, unsigned char **encodedChars, unsigned int *encodedCharsLen, unsigned int &b);
unsigned char* encodePattern(unsigned char* pattern, unsigned int patternLen, unsigned char** encodedChars, unsigned int* encodedCharsLen, unsigned int &encodedPatternLen, bool &wrongEncoding);
unsigned char *encode125(unsigned char* text, unsigned int textLen, unsigned int *selectedOrdChars, unsigned int &encodedTextLen);
unsigned int occ(unsigned int *a, unsigned int aLen, unsigned int elem);
void fill125LUT(unsigned int *selectedOrdChars, unsigned int lut[][125]);
unsigned char *buildRank_512_enc125(unsigned char *bwtEnc125, unsigned int bwtEnc125Len, unsigned int lut[][125], unsigned int &raw_bwtWithRanksLen);
unsigned char *buildRank_1024_enc125(unsigned char *bwtEnc125, unsigned int bwtEnc125Len, unsigned int lut[][125], unsigned int &raw_bwtWithRanksLen);
unsigned int getRank_512_enc125(unsigned char c, unsigned int i, unsigned char *bwtWithRanks, unsigned int lut[][125]);
unsigned int count_512_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal);
unsigned int getRank_1024_enc125(unsigned char c, unsigned int i, unsigned char *bwtWithRanks, unsigned int lut[][125]);
unsigned int count_1024_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal);
#endif /* FMDUMMY_H_ */
