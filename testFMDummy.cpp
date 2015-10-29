#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <map>
#include "shared/common.h"
#include "shared/patterns.h"
#include "shared/timer.h"
#include "fmdummy.h"

using namespace std;
using namespace fmdummy;

CStopWatch timer;

map<string, FMDummy1::IndexType> FMDummy1IndexTypesMap = {{"256", FMDummy1::TYPE_256}, {"512", FMDummy1::TYPE_512}};
map<string, vector<unsigned char>> FMDummy1SelectedCharsMap = {{"ACGT", {65, 67, 71, 84}}, {"all", {}}};
map<string, FMDummy2::IndexType> FMDummy2IndexTypesMap = {{"256", FMDummy2::TYPE_256}, {"512", FMDummy2::TYPE_512}};
map<string, FMDummy2::Schema> FMDummy2SchemaMap = {{"SCBO", FMDummy2::SCHEMA_SCBO}, {"CB", FMDummy2::SCHEMA_CB}};
map<string, FMDummy2::BitsPerChar> FMDummy2BitsPerCharMap = {{"3", FMDummy2::BITS_3}, {"4", FMDummy2::BITS_4}};
map<string, FMDummy3::IndexType> FMDummy3IndexTypesMap = {{"512", FMDummy3::TYPE_512}, {"1024", FMDummy3::TYPE_1024}};
map<string, FMDummyWT::IndexType> FMDummyWTIndexTypesMap = {{"512", FMDummyWT::TYPE_512}, {"1024", FMDummyWT::TYPE_1024}};
map<string, FMDummyWT::WTType> FMDummyWTWTTypeMap = {{"2", FMDummyWT::TYPE_WT2}, {"4", FMDummyWT::TYPE_WT4}, {"8", FMDummyWT::TYPE_WT8}};

void fmDummy1(string indexType, string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy1hash(string indexType, string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2(string indexType, string encodedSchema, string bits, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2hash(string indexType, string encodedSchema, string bits, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3hash(string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyWT(string wtType, string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyWThash(string wtType, string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test:" << endl;
	cout << "FMDummy1: ./" << argv[0] << " 1 256|512 all|ACGT fileName q m" << endl;
	cout << "FMDummy1hash: ./" << argv[0] << " 1hash 256|512 k loadFactor all|ACGT fileName q m" << endl;
	cout << "FMDummy2: ./" << argv[0] << " 2 256|512 SCBO|CB 3|4 fileName q m" << endl;
	cout << "FMDummy2hash: ./" << argv[0] << " 2hash 256|512 SCBO|CB 3|4 k loadFactor fileName q m" << endl;
	cout << "FMDummy3: ./" << argv[0] << " 3 512|1024 fileName q m" << endl;
	cout << "FMDummy3hash: ./" << argv[0] << " 3 512|1024 k loadFactor fileName q m" << endl;
	cout << "FMDummyWT: ./" << argv[0] << " WT 2|4|8 512|1024 fileName q m" << endl;
	cout << "FMDummyWThash: ./" << argv[0] << " WThash 2|4|8 k loadFactor 512|1024 fileName q m" << endl;
	cout << "where:" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "q - number of patterns (queries)" << endl;
	cout << "m - pattern length" << endl;
	cout << "k - suffix length to be hashed (k > 0)" << endl;
	cout << "loadFactor - load factor to hash table (range: (0.0, 1.0))" << endl << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		getUsage(argv);
		return 1;
	}

	if ((string)argv[1] == "1") {
		if (argc < 7 || FMDummy1IndexTypesMap.find(string(argv[2])) == FMDummy1IndexTypesMap.end() || FMDummy1SelectedCharsMap.find(string(argv[3])) == FMDummy1SelectedCharsMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummy1(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	}
	else if ((string)argv[1] == "1hash") {
		if (argc < 9 || FMDummy1IndexTypesMap.find(string(argv[2])) == FMDummy1IndexTypesMap.end() || FMDummy1SelectedCharsMap.find(string(argv[3])) == FMDummy1SelectedCharsMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummy1hash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), argv[6], atoi(argv[7]), atoi(argv[8]));
	}
	else if ((string)argv[1] == "2") {
		if (argc < 8 || FMDummy2IndexTypesMap.find(string(argv[2])) == FMDummy2IndexTypesMap.end() || FMDummy2SchemaMap.find(string(argv[3])) == FMDummy2SchemaMap.end() || FMDummy2BitsPerCharMap.find(string(argv[4])) == FMDummy2BitsPerCharMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummy2(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	}
	else if ((string)argv[1] == "2hash") {
		if (argc < 10 || FMDummy2IndexTypesMap.find(string(argv[2])) == FMDummy2IndexTypesMap.end() || FMDummy2SchemaMap.find(string(argv[3])) == FMDummy2SchemaMap.end() || FMDummy2BitsPerCharMap.find(string(argv[4])) == FMDummy2BitsPerCharMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummy2hash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), string(argv[6]), argv[7], atoi(argv[8]), atoi(argv[9]));
	}
	else if ((string)argv[1] == "3") {
		if (argc < 6 || FMDummy3IndexTypesMap.find(string(argv[2])) == FMDummy3IndexTypesMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummy3(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	}
	else if ((string)argv[1] == "3hash") {
		if (argc < 8 || FMDummy3IndexTypesMap.find(string(argv[2])) == FMDummy3IndexTypesMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummy3hash(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	}
	else if ((string)argv[1] == "WT") {
		if (argc < 7 || FMDummyWTWTTypeMap.find(string(argv[2])) == FMDummyWTWTTypeMap.end() || FMDummyWTIndexTypesMap.find(string(argv[3])) == FMDummyWTIndexTypesMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummyWT(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	}
	else if ((string)argv[1] == "WThash") {
		if (argc < 9 || FMDummyWTWTTypeMap.find(string(argv[2])) == FMDummyWTWTTypeMap.end() || FMDummyWTIndexTypesMap.find(string(argv[3])) == FMDummyWTIndexTypesMap.end()) {
			getUsage(argv);
			exit(1);
		}
		fmDummyWThash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), argv[6], atoi(argv[7]), atoi(argv[8]));
	}
	else {
		getUsage(argv);
		exit(1);
	}
}

void fmDummy1(string indexType, string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy1 *FMD1;
	string indexFileNameString = string("FMD1-") + (string)textFileName + "-" + indexType + "-" + selectedChars + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(FMDummy1IndexTypesMap[indexType], FMDummy1SelectedCharsMap[selectedChars]);
		FMD1->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD1->build(text, textLen);
		FMD1->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m, FMDummy1SelectedCharsMap[selectedChars]);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD1->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy1.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD1->getIndexSize() / (double)FMD1->getTextSize();
	cout << "count FMDummy1_" << indexType << "_" << selectedChars << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << selectedChars << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMD1;
	delete P;
}

void fmDummy1hash(string indexType, string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy1 *FMD1;
	string indexFileNameString = string("FMD1hash-") + (string)textFileName + "-" + indexType + "-" + selectedChars + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(FMDummy1IndexTypesMap[indexType], FMDummy1SelectedCharsMap[selectedChars], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMD1->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD1->build(text, textLen);
		FMD1->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m, FMDummy1SelectedCharsMap[selectedChars]);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD1->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy1hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD1->getIndexSize() / (double)FMD1->getTextSize();
	cout << "count FMDummy1hash_" << indexType << "_" << selectedChars << "_" << k << "_" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << selectedChars << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMD1;
	delete P;
}

void fmDummy2(string indexType, string encodedSchema, string bits, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy2 *FMD2;
	string indexFileNameString = string("FMD2-") + (string)textFileName + "-" + indexType + "-" + encodedSchema + "-" + bits + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2 = new FMDummy2();
		FMD2->load(indexFileName);
	} else {
		FMD2 = new FMDummy2(FMDummy2IndexTypesMap[indexType], FMDummy2SchemaMap[encodedSchema], FMDummy2BitsPerCharMap[bits]);
		FMD2->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD2->build(text, textLen);
		FMD2->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD2->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy2.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD2->getIndexSize() / (double)FMD2->getTextSize();
	cout << "count FMDummy2_" << indexType << "_" << encodedSchema << "_" << bits << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << encodedSchema << " " << bits << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMD2;
	delete P;
}

void fmDummy2hash(string indexType, string encodedSchema, string bits, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy2 *FMD2;
	string indexFileNameString = string("FMD2hash-") + (string)textFileName + "-" + indexType + "-" + encodedSchema + "-" + bits + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2 = new FMDummy2();
		FMD2->load(indexFileName);
	} else {
		FMD2 = new FMDummy2(FMDummy2IndexTypesMap[indexType], FMDummy2SchemaMap[encodedSchema], FMDummy2BitsPerCharMap[bits], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMD2->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD2->build(text, textLen);
		FMD2->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD2->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy2hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD2->getIndexSize() / (double)FMD2->getTextSize();
	cout << "count FMDummy2hash_" << indexType << "_" << encodedSchema << "_" << bits << "_" << k << "_" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << encodedSchema << " " << bits << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMD2;
	delete P;
}

void fmDummy3(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy3 *FMD3;
	string indexFileNameString = string("FMD3-") + (string)textFileName + "-" + indexType + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3 = new FMDummy3();
		FMD3->load(indexFileName);
	} else {
		FMD3 = new FMDummy3(FMDummy3IndexTypesMap[indexType]);
		FMD3->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD3->build(text, textLen);
		FMD3->save(indexFileName);
	}

	vector<unsigned char> selectedChars = {'A', 'C', 'G', 'T'};
	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD3->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy3.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD3->getIndexSize() / (double)FMD3->getTextSize();
	cout << "count FMDummy3_" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMD3;
	delete P;
}

void fmDummy3hash(string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy3 *FMD3;
	string indexFileNameString = string("FMD3hash-") + (string)textFileName + "-" + indexType + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3 = new FMDummy3();
		FMD3->load(indexFileName);
	} else {
		FMD3 = new FMDummy3(FMDummy3IndexTypesMap[indexType], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMD3->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD3->build(text, textLen);
		FMD3->save(indexFileName);
	}

	vector<unsigned char> selectedChars = {'A', 'C', 'G', 'T'};
	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD3->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy3hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD3->getIndexSize() / (double)FMD3->getTextSize();
	cout << "count FMDummy3hash_" << indexType << "_" << k << "_" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << indexType << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMD3;
	delete P;
}

void fmDummyWT(string wtType, string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummyWT *FMDWT;
	string indexFileNameString = string("FMDWT-") + (string)textFileName + "-" + wtType + "-" + indexType + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDWT = new FMDummyWT();
		FMDWT->load(indexFileName);
	} else {
		FMDWT = new FMDummyWT(FMDummyWTWTTypeMap[wtType], FMDummyWTIndexTypesMap[indexType]);
		FMDWT->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMDWT->build(text, textLen);
		FMDWT->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMDWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDWT->getIndexSize() / (double)FMDWT->getTextSize();
	cout << "count FMDummyWT" << wtType << "_" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << wtType << " " << indexType << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMDWT;
	delete P;
}

void fmDummyWThash(string wtType, string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummyWT *FMDWT;
	string indexFileNameString = string("FMDWThash-") + (string)textFileName + "-" + wtType + "-" + indexType + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDWT = new FMDummyWT();
		FMDWT->load(indexFileName);
	} else {
		FMDWT = new FMDummyWT(FMDummyWTWTTypeMap[wtType], FMDummyWTIndexTypesMap[indexType], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMDWT->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMDWT->build(text, textLen);
		FMDWT->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMDWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyWThash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDWT->getIndexSize() / (double)FMDWT->getTextSize();
	cout << "count FMDummyWT" << wtType << "hash_" << indexType << "_" << k << "_" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " " << wtType << " " << indexType << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << "DIFFERENCES: " << differences << endl;
		resultFile << " DIFFERENCES: " << differences;
	} else {
		cout << "Differences: " << differences << endl;
	}
	resultFile << endl;
	resultFile.close();

	if (text != NULL) delete[] text;
	delete[] indexCounts;
	delete FMDWT;
	delete P;
}
