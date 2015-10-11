#ifndef SHARED_HASH_H_
#define SHARED_HASH_H_

class HT {
private:
	void freeMemory();
	void initialize();
	void setLoadFactor(double loadFactor);
	void setK(unsigned int k);
	unsigned int getUniqueSuffixNum(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen);
	void fillHTData(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen);
	void fillHTDataWithEntries(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen);

public:
	double loadFactor;
	unsigned int k;

	unsigned int bucketsNum;
	unsigned int emptyValueHT;

	unsigned int lut2[256][257];
	unsigned int *boundariesHT;
	unsigned int *alignedBoundariesHT;
	unsigned char *entriesHT;
	unsigned char *alignedEntriesHT;

	HT() {
		this->initialize();
	}

	HT(unsigned int k, double loadFactor) {
		this->initialize();
		this->setK(k);
		this->setLoadFactor(loadFactor);
	};

	unsigned int getHTSize();
	unsigned int getHashValue(unsigned char* str);
	void build(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen);
	void buildWithEntries(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen);
	void getBoundaries(unsigned char *pattern, unsigned char *text, unsigned int *sa, unsigned int &leftBoundary, unsigned int &rightBoundary);
	void getBoundariesWithEntries(unsigned char *pattern, unsigned int &leftBoundary, unsigned int &rightBoundary);
	void save(FILE *outFile);
	void load(FILE *inFile);
	void free();

	~HT() {
		this->freeMemory();
	}
};

#endif /* SHARED_HASH_H_ */
