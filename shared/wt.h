#ifndef SHARED_WT_H_
#define SHARED_WT_H_

namespace fmdummy {

class WT {
private:
	void freeMemory();
	void initialize();

public:
	unsigned long long *bits;
	unsigned int bitsLen;
	unsigned long long *alignedBits;
	WT** nodes;
	unsigned int nodesLen;

	WT() {
		this->initialize();
	}

	WT(int wtType) {
		this->initialize();
		this->nodesLen = wtType;
		this->nodes = new WT *[this->nodesLen];
	};

	unsigned int getWTSize();
	void save(FILE *outFile);
	void load(FILE *inFile);
	void free();

	~WT() {
		this->freeMemory();
	}
};

}

#endif /* SHARED_WT_H_ */
