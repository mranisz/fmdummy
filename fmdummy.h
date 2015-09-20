#include "shared/api.h"
#include <cstdio>
#include <string>

using namespace std;

#ifndef FMDUMMY_H_
#define FMDUMMY_H_

//FMDUMMY1

class FMDummy1 : public I_Api {
private:
	unsigned long long *raw_bwtWithRanks[256];
	unsigned long long **bwtWithRanks;

	unsigned int raw_bwtWithRanksLen;

	int type;
	bool allChars;
	unsigned int ordCharsLen;
	unsigned int *ordChars;
	unsigned int *c;

	unsigned int textSize;

	unsigned long long **(*builder)(unsigned long long **, unsigned int, unsigned long long **, unsigned int *, unsigned int, unsigned int &);
	unsigned int (*countOperation)(unsigned char*, unsigned int, unsigned int*, unsigned long long**);

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

unsigned char *getBinDenseForChar(unsigned char *bwt, unsigned int bwtLen, int ordChar);
unsigned long long** buildRank_64_256(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned long long** buildRank_64_256_counter48(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned long long** buildRank_64_512(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned long long** buildRank_64_512_counter40(unsigned long long** bwtInLong, unsigned int bwtInLongLen, unsigned long long** raw_bwtWithRanks, unsigned int *ordChars, unsigned int ordCharsLen, unsigned int &raw_bwtWithRanksLen);
unsigned int getRank_64_256(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_64_256(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks);
unsigned int getRank_64_256_counter48(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_64_256_counter48(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks);
unsigned int getRank_64_512(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_64_512(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks);
unsigned int getRank_64_512_counter40(unsigned char c, unsigned int i, unsigned long long** bwtWithRanks);
unsigned int count_64_512_counter40(unsigned char *Q, unsigned int m, unsigned int *C, unsigned long long** bwtWithRanks);

#endif /* FMDUMMY_H_ */
