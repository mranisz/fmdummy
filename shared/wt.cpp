#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "wt.h"

using namespace std;

namespace fmdummy {

unsigned int WT::getWTSize() {
	unsigned int size = sizeof(this->bitsLen) + sizeof(this->nodesLen) + sizeof(unsigned long long *);
	size += ((this->bitsLen + 16) * sizeof(unsigned long long) + this->nodesLen * sizeof(WT *));
	for (unsigned int i = 0; i < this->nodesLen; ++i) if (this->nodes[i] != NULL) size += this->nodes[i]->getWTSize();
	return size;
}

void WT::initialize() {
	this->bits = NULL;
	this->bitsLen = 0;
	this->alignedBits = NULL;
	this->nodes = NULL;
	this->nodesLen = 0;
}

void WT::freeMemory() {
	for (unsigned int i = 0; i < this->nodesLen; ++i) if (this->nodes[i] != NULL) this->nodes[i]->freeMemory();
	if (this->nodes != NULL) delete[] this->nodes;
	if (this->bits != NULL) delete[] this->bits;
}

void WT::free() {
	this->freeMemory();
	this->initialize();
}

void WT::save(FILE *outFile) {
	bool nullNode = false;
	bool notNullNode = true;
	fwrite(&this->bitsLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	if (this->bitsLen > 0) fwrite(this->alignedBits, (size_t)sizeof(unsigned long long), (size_t)this->bitsLen, outFile);
	fwrite(&this->nodesLen, (size_t)sizeof(unsigned int), (size_t)1, outFile);
	for (unsigned int i = 0; i < this->nodesLen; ++i) {
		if (this->nodes[i] == NULL) fwrite(&nullNode, (size_t)sizeof(bool), (size_t)1, outFile);
		else {
			fwrite(&notNullNode, (size_t)sizeof(bool), (size_t)1, outFile);
			this->nodes[i]->save(outFile);
		}
	}
}

void WT::load(FILE *inFile) {
	this->free();
	bool isNotNullNode;
	size_t result;
	result = fread(&this->bitsLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	if (this->bitsLen > 0) {
		this->bits = new unsigned long long[this->bitsLen + 16];
		this->alignedBits = this->bits;
		while ((unsigned long long)(this->alignedBits) % 128) ++(this->alignedBits);
		result = fread(this->alignedBits, (size_t)sizeof(unsigned long long), (size_t)this->bitsLen, inFile);
		if (result != this->bitsLen) {
			cout << "Error loading index" << endl;
			exit(1);
		}
	}
	result = fread(&this->nodesLen, (size_t)sizeof(unsigned int), (size_t)1, inFile);
	if (result != 1) {
		cout << "Error loading index" << endl;
		exit(1);
	}
	this->nodes = new WT *[this->nodesLen];
	for (unsigned int i = 0; i < this->nodesLen; ++i) {
		result = fread(&isNotNullNode, (size_t)sizeof(bool), (size_t)1, inFile);
		if (result != 1) {
			cout << "Error loading index" << endl;
			exit(1);
		}
		if (isNotNullNode) {
			this->nodes[i] = new WT();
			this->nodes[i]->load(inFile);
		} else this->nodes[i] = NULL;
	}
}

}
