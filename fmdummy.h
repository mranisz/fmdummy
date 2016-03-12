#ifndef FMDUMMY_H_
#define FMDUMMY_H_

#include "shared/common.h"
#include "shared/wt.h"
#include "shared/hash.h"
#include "shared/huff.h"
#include <cstdio>
#include <vector>

using namespace std;

namespace fmdummy {

/*FMDUMMY1*/

class FMDummy1 : public Index {
private:
	alignas(128) unsigned long long *bwtWithRanks[256];
	alignas(128) unsigned long long *alignedBWTWithRanks[256];
	unsigned int bwtWithRanksLen;
	alignas(128) unsigned int c[257];
	HTExt *ht = NULL;

	int type;
	vector<unsigned char> selectedChars;
	bool allChars;

	unsigned int textLen;

	void (*builder)(unsigned long long **, unsigned int, vector<unsigned char>, unsigned long long **, unsigned int &, unsigned long long **) = NULL;
	unsigned int (FMDummy1::*countOperation)(unsigned char *, unsigned int) = NULL;

	void freeMemory();
	void initialize();
	void setType(int indexType);
	void setSelectedChars(vector<unsigned char> selectedChars);
	void setFunctions();
	unsigned int count_std_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_256_counter48(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_512_counter40(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_512_counter40(unsigned char *pattern, unsigned int patternLen);

public:
	enum IndexType {
		TYPE_256 = 1,
		TYPE_512 = 2
	};
	FMDummy1() {
		this->initialize();
                this->setType(FMDummy1::TYPE_256);
		this->setSelectedChars({});
		this->setFunctions();
	}

	FMDummy1(FMDummy1::IndexType indexType, vector<unsigned char> selectedChars) {
		this->initialize();
		this->setType(indexType);
		this->setSelectedChars(selectedChars);
		this->setFunctions();
	}

	FMDummy1(FMDummy1::IndexType indexType, vector<unsigned char> selectedChars, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(indexType);
		this->setSelectedChars(selectedChars);
                this->ht = new HTExt(HTExt::STANDARD, k, loadFactor);
		this->setFunctions();
	}

	~FMDummy1() {
		this->freeMemory();
                if (this->ht != NULL) delete this->ht;
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(const char *fileName);
	void load(const char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

/*FMDUMMY2*/

class FMDummy2 : public Index {
private:
	alignas(128) unsigned long long *bwtWithRanks[256];
	alignas(128) unsigned long long *alignedBWTWithRanks[256];
	unsigned int bwtWithRanksLen;
	unsigned char *encodedChars;
	alignas(128) unsigned int encodedCharsLen[256];
	unsigned int maxEncodedCharsLen;
	alignas(128) unsigned int c[257];
	unsigned int bInC;
	unsigned char *encodedPattern;
	unsigned int maxPatternLen;
	HTExt *ht = NULL;

	int type;
	int schema;
	int bitsPerChar;

	unsigned int textLen;

	void (*builder)(unsigned long long **, unsigned int, vector<unsigned char>, unsigned long long **, unsigned int &, unsigned long long **) = NULL;
	unsigned int (FMDummy2::*countOperation)(unsigned char *, unsigned int) = NULL;

	void freeMemory();
	void initialize();
	void setType(int indexType, int schema);
	void setBitsPerChar(int bitsPerChar);
	void setMaxEncodedCharsLen();
	void setEncodedPattern(unsigned int maxPatternLen);
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
	enum IndexType {
		TYPE_256 = 1,
		TYPE_512 = 2
	};

	enum BitsPerChar {
		BITS_3 = 3,
		BITS_4 = 4
	};

	enum Schema {
		SCHEMA_SCBO = 1,
		SCHEMA_CB = 2
	};

	FMDummy2() {
		this->initialize();
                this->setType(FMDummy2::TYPE_256, FMDummy2::SCHEMA_SCBO);
		this->setBitsPerChar(FMDummy2::BITS_4);
		this->setFunctions();
	}

	FMDummy2(FMDummy2::IndexType indexType, FMDummy2::Schema schema, FMDummy2::BitsPerChar bitsPerChar) {
		this->initialize();
		this->setType(indexType, schema);
		this->setBitsPerChar(bitsPerChar);
		this->setFunctions();
	}

	FMDummy2(FMDummy2::IndexType indexType, FMDummy2::Schema schema, FMDummy2::BitsPerChar bitsPerChar, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(indexType, schema);
		this->setBitsPerChar(bitsPerChar);
		this->ht = new HTExt(HTExt::STANDARD, k, loadFactor);
		this->setFunctions();
	}

	~FMDummy2() {
		this->freeMemory();
                if (this->ht != NULL) delete this->ht;
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(const char *fileName);
	void load(const char *fileName);
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
	unsigned char *alignedBWTWithRanks;
	unsigned int bwtWithRanksLen;
	alignas(128) unsigned int lut[256][125];
	alignas(128) unsigned int c[257];
	HTExt *ht = NULL;

	int type;

	unsigned int textLen;

	unsigned int (FMDummy3::*countOperation)(unsigned char *, unsigned int) = NULL;

	void freeMemory();
	void initialize();
	void setType(int indexType);
	void setFunctions();
	void buildRank_512_enc125(unsigned char *bwtEnc125, unsigned int bwtLen);
	void buildRank_1024_enc125(unsigned char *bwtEnc125, unsigned int bwtLen);
	unsigned int count_std_512_enc125(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_std_1024_enc125(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_512_enc125(unsigned char *pattern, unsigned int patternLen);
	unsigned int count_hash_1024_enc125(unsigned char *pattern, unsigned int patternLen);

public:
	enum IndexType {
		TYPE_512 = 1,
		TYPE_1024 = 2
	};

	FMDummy3() {
		this->initialize();
                this->setType(FMDummy3::TYPE_512);
		this->setFunctions();
	}

	FMDummy3(FMDummy3::IndexType indexType) {
		this->initialize();
		this->setType(indexType);
		this->setFunctions();
	}

	FMDummy3(FMDummy3::IndexType indexType, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(indexType);
		this->ht = new HTExt(HTExt::STANDARD, k, loadFactor);
		this->setFunctions();
	}

	~FMDummy3() {
		this->freeMemory();
                if (this->ht != NULL) delete this->ht;
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(const char *fileName);
	void load(const char *fileName);
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
	alignas(128) unsigned long long code[256];
	alignas(128) unsigned int codeLen[256];
	alignas(128) unsigned int c[257];
	HTExt *ht = NULL;

	int type;
	int wtType;

	unsigned int textLen;

	unsigned int (FMDummyWT::*countOperation)(unsigned char *, unsigned int) = NULL;

	void freeMemory();
	void initialize();
	void setType(int wtType, int indexType);
	void setFunctions();
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
	enum WTType {
		TYPE_WT2 = 2,
		TYPE_WT4 = 4,
		TYPE_WT8 = 8
	};

	enum IndexType {
		TYPE_512 = 8,
		TYPE_1024 = 16
	};

	FMDummyWT() {
		this->initialize();
                this->setType(FMDummyWT::TYPE_WT2, FMDummyWT::TYPE_512);
		this->setFunctions();
	}

	FMDummyWT(FMDummyWT::WTType wtType, FMDummyWT::IndexType indexType) {
		this->initialize();
		this->setType(wtType, indexType);
		this->setFunctions();
	}

	FMDummyWT(FMDummyWT::WTType wtType, FMDummyWT::IndexType indexType, unsigned int k, double loadFactor) {
		this->initialize();
		this->setType(wtType, indexType);
		this->ht = new HTExt(HTExt::STANDARD, k, loadFactor);
		this->setFunctions();
	}

	~FMDummyWT() {
		this->freeMemory();
                if (this->ht != NULL) delete this->ht;
	}

	void build(unsigned char *text, unsigned int textLen);
	void save(const char *fileName);
	void load(const char *fileName);
	void free();
	unsigned int getIndexSize();
	unsigned int getTextSize();

	unsigned int count(unsigned char *pattern, unsigned int patternLen);
	unsigned int *locate(unsigned char *pattern, unsigned int patternLen);
};

/*SHARED STUFF*/

unsigned char *getBinDenseForChar(unsigned char *bwt, unsigned int bwtLen, int ordChar);
void buildRank_256_counter48(unsigned long long **bwtInLong, unsigned int bwtInLongLen, vector<unsigned char> selectedChars, unsigned long long **bwtWithRanks, unsigned int &bwtWithRanksLen, unsigned long long **alignedBWTWithRanks);
void buildRank_512_counter40(unsigned long long **bwtInLong, unsigned int bwtInLongLen, vector<unsigned char> selectedChars, unsigned long long **bwtWithRanks, unsigned int &bwtWithRanksLen, unsigned long long **alignedBWTWithRanks);
unsigned int count_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long **bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
void getCountBoundaries_256_counter48(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long **bwtWithRanks, unsigned int firstVal, unsigned int lastVal, unsigned int &leftBoundary, unsigned int &rightBoundary);
unsigned int count_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long **bwtWithRanks, unsigned int firstVal, unsigned int lastVal);
void getCountBoundaries_512_counter40(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned long long **bwtWithRanks, unsigned int firstVal, unsigned int lastVal, unsigned int &leftBoundary, unsigned int &rightBoundary);
unsigned char *encode125(unsigned char* text, unsigned int textLen, vector<unsigned char> selectedChars, unsigned int &encodedTextLen);
void fill125LUT(vector<unsigned char> selectedChars, unsigned int lut[][125]);
unsigned int count_512_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal);
unsigned int count_1024_enc125(unsigned char *pattern, unsigned int i, unsigned int *C, unsigned char *bwtWithRanks, unsigned int lut[][125], unsigned int firstVal, unsigned int lastVal);

}

#endif /* FMDUMMY_H_ */
