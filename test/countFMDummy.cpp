#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <map>
#include "../shared/patterns.h"
#include "../shared/timer.h"
#include "../fmdummy.h"

using namespace std;
using namespace shared;
using namespace fmdummy;

ChronoStopWatch timer;

map<string, FMDummy1::IndexType> FMDummy1IndexTypesMap = {{"256", FMDummy1::TYPE_256}, {"512", FMDummy1::TYPE_512}};
map<string, vector<unsigned char>> FMDummy1SelectedCharsMap = {{"ACGT", {65, 67, 71, 84}}, {"all", {}}};
map<string, FMDummy2::IndexType> FMDummy2IndexTypesMap = {{"256", FMDummy2::TYPE_256}, {"512", FMDummy2::TYPE_512}};
map<string, FMDummy2::Schema> FMDummy2SchemaMap = {{"SCBO", FMDummy2::SCHEMA_SCBO}, {"CB", FMDummy2::SCHEMA_CB}};
map<string, FMDummy2::BitsPerChar> FMDummy2BitsPerCharMap = {{"3", FMDummy2::BITS_3}, {"4", FMDummy2::BITS_4}};
map<string, FMDummy3::IndexType> FMDummy3IndexTypesMap = {{"512", FMDummy3::TYPE_512}, {"1024", FMDummy3::TYPE_1024}};
map<string, FMDummyWT::WTType> FMDummyWTWTTypeMap = {{"2", FMDummyWT::TYPE_WT2}, {"4", FMDummyWT::TYPE_WT4}, {"8", FMDummyWT::TYPE_WT8}};
map<string, FMDummyWT::IndexType> FMDummyWTIndexTypesMap = {{"512", FMDummyWT::TYPE_512}, {"1024", FMDummyWT::TYPE_1024}};

void fmDummy1(string indexType, string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy1Hash(string indexType, string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2(string indexType, string encodedSchema, string bits, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash(string indexType, string encodedSchema, string bits, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3Hash(string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyWT(string wtType, string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyWTHash(string wtType, string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test (count):" << endl;
	cout << "FMDummy1: " << argv[0] << " 1 256|512 all|ACGT fileName patternNum patternLen" << endl;
	cout << "FMDummy1-hash: " << argv[0] << " 1 256|512 all|ACGT k loadFactor fileName patternNum patternLen" << endl;
        cout << "FMDummy2: " << argv[0] << " 2 256|512 SCBO|CB 3|4 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash: " << argv[0] << " 2 256|512 SCBO|CB 3|4 k loadFactor fileName patternNum patternLen" << endl;
        cout << "FMDummy3: " << argv[0] << " 3 512|1024 fileName patternNum patternLen" << endl;
	cout << "FMDummy3-hash: " << argv[0] << " 3 512|1024 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyWT: " << argv[0] << " WT 2|4|8 512|1024 fileName patternNum patternLen" << endl;
	cout << "FMDummyWT-hash: " << argv[0] << " WT 2|4|8 512|1024 k loadFactor fileName patternNum patternLen" << endl;
	cout << "where:" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "patternNum - number of patterns" << endl;
	cout << "patternLen - pattern length" << endl;
	cout << "k - suffix length to be hashed (k > 0)" << endl;
	cout << "loadFactor - load factor of hash table (range: (0.0, 1.0))" << endl << endl;
}

int main(int argc, char *argv[]) {
	if (argc < 5) {
		getUsage(argv);
		exit(1);
	}

	if ((string)argv[1] == "1") {
                if (FMDummy1IndexTypesMap.find(string(argv[2])) != FMDummy1IndexTypesMap.end() && FMDummy1SelectedCharsMap.find(string(argv[3])) != FMDummy1SelectedCharsMap.end()) {
                    if (argc == 7) fmDummy1(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));    
                    else if (argc == 9) fmDummy1Hash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), argv[6], atoi(argv[7]), atoi(argv[8]));
		}
        }
	else if ((string)argv[1] == "2") {
		if (FMDummy2IndexTypesMap.find(string(argv[2])) != FMDummy2IndexTypesMap.end() && FMDummy2SchemaMap.find(string(argv[3])) != FMDummy2SchemaMap.end() && FMDummy2BitsPerCharMap.find(string(argv[4])) != FMDummy2BitsPerCharMap.end()) {
                        if (argc == 8) fmDummy2(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
                        else if (argc == 10) fmDummy2Hash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), string(argv[6]), argv[7], atoi(argv[8]), atoi(argv[9]));
		}
	}
	else if ((string)argv[1] == "3") {
		if (FMDummy3IndexTypesMap.find(string(argv[2])) != FMDummy3IndexTypesMap.end()) {
                        if (argc == 6) fmDummy3(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
                        else if (argc == 8) fmDummy3Hash(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
		}
	}
	else if ((string)argv[1] == "WT") {
		if (FMDummyWTWTTypeMap.find(string(argv[2])) != FMDummyWTWTTypeMap.end() && FMDummyWTIndexTypesMap.find(string(argv[3])) != FMDummyWTIndexTypesMap.end()) {
                        if (argc == 7) fmDummyWT(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
                        else if (argc == 9) fmDummyWTHash(string(argv[2]), string(argv[3]), string(argv[4]), string(argv[5]), argv[6], atoi(argv[7]), atoi(argv[8]));
		}
	}
        getUsage(argv);
        exit(1);
}

void fmDummy1(string indexType, string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy1 *FMD1;
	string indexFileNameString = "FMD1-" + (string)textFileName + "-" + indexType + "-" + selectedChars + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(FMDummy1IndexTypesMap[indexType], FMDummy1SelectedCharsMap[selectedChars]);
		FMD1->setVerbose(true);
		FMD1->build(textFileName);
		FMD1->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m, FMDummy1SelectedCharsMap[selectedChars]);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m, FMDummy1SelectedCharsMap[selectedChars]);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
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
	cout << "count FMDummy1-" << indexType << "-" << selectedChars << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMD1;
	delete P;
        exit(0);
}

void fmDummy1Hash(string indexType, string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy1 *FMD1;
	string indexFileNameString = "FMD1-hash-" + (string)textFileName + "-" + indexType + "-" + selectedChars + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(FMDummy1IndexTypesMap[indexType], FMDummy1SelectedCharsMap[selectedChars], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMD1->setVerbose(true);
		FMD1->build(textFileName);
		FMD1->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m, FMDummy1SelectedCharsMap[selectedChars]);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m, FMDummy1SelectedCharsMap[selectedChars]);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD1->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy1-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD1->getIndexSize() / (double)FMD1->getTextSize();
	cout << "count FMDummy1-hash-" << indexType << "-" << selectedChars << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMD1;
	delete P;
        exit(0);
}

void fmDummy2(string indexType, string encodedSchema, string bits, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2 *FMD2;
	string indexFileNameString = "FMD2-" + (string)textFileName + "-" + indexType + "-" + encodedSchema + "-" + bits + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2 = new FMDummy2();
		FMD2->load(indexFileName);
	} else {
		FMD2 = new FMDummy2(FMDummy2IndexTypesMap[indexType], FMDummy2SchemaMap[encodedSchema], FMDummy2BitsPerCharMap[bits]);
		FMD2->setVerbose(true);
		FMD2->build(textFileName);
		FMD2->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
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
	cout << "count FMDummy2-" << indexType << "-" << encodedSchema << "-" << bits << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMD2;
	delete P;
        exit(0);
}

void fmDummy2Hash(string indexType, string encodedSchema, string bits, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2 *FMD2;
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-" + indexType + "-" + encodedSchema + "-" + bits + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2 = new FMDummy2();
		FMD2->load(indexFileName);
	} else {
		FMD2 = new FMDummy2(FMDummy2IndexTypesMap[indexType], FMDummy2SchemaMap[encodedSchema], FMDummy2BitsPerCharMap[bits], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMD2->setVerbose(true);
		FMD2->build(textFileName);
		FMD2->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD2->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy2-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD2->getIndexSize() / (double)FMD2->getTextSize();
	cout << "count FMDummy2-hash-" << indexType << "-" << encodedSchema << "-" << bits << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMD2;
	delete P;
        exit(0);
}

void fmDummy3(string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy3 *FMD3;
	string indexFileNameString = "FMD3-" + (string)textFileName + "-" + indexType + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3 = new FMDummy3();
		FMD3->load(indexFileName);
	} else {
		FMD3 = new FMDummy3(FMDummy3IndexTypesMap[indexType]);
		FMD3->setVerbose(true);
		FMD3->build(textFileName);
		FMD3->save(indexFileName);
	}

	vector<unsigned char> selectedChars = {'A', 'C', 'G', 'T'};
	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m, selectedChars);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
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
	cout << "count FMDummy3-" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMD3;
	delete P;
        exit(0);
}

void fmDummy3Hash(string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy3 *FMD3;
	string indexFileNameString = "FMD3-hash-" + (string)textFileName + "-" + indexType + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3 = new FMDummy3();
		FMD3->load(indexFileName);
	} else {
		FMD3 = new FMDummy3(FMDummy3IndexTypesMap[indexType], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMD3->setVerbose(true);
		FMD3->build(textFileName);
		FMD3->save(indexFileName);
	}

	vector<unsigned char> selectedChars = {'A', 'C', 'G', 'T'};
	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m, selectedChars);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD3->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummy3-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMD3->getIndexSize() / (double)FMD3->getTextSize();
	cout << "count FMDummy3-hash-" << indexType << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMD3;
	delete P;
        exit(0);
}

void fmDummyWT(string wtType, string indexType, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyWT *FMDWT;
	string indexFileNameString = "FMDWT-" + (string)textFileName + "-" + wtType + "-" + indexType + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDWT = new FMDummyWT();
		FMDWT->load(indexFileName);
	} else {
		FMDWT = new FMDummyWT(FMDummyWTWTTypeMap[wtType], FMDummyWTIndexTypesMap[indexType]);
		FMDWT->setVerbose(true);
		FMDWT->build(textFileName);
		FMDWT->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
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
	cout << "count FMDummyWT-" << wtType << "-" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMDWT;
	delete P;
        exit(0);
}

void fmDummyWTHash(string wtType, string indexType, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyWT *FMDWT;
	string indexFileNameString = "FMDWT-hash-" + (string)textFileName + "-" + wtType + "-" + indexType + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDWT = new FMDummyWT();
		FMDWT->load(indexFileName);
	} else {
		FMDWT = new FMDummyWT(FMDummyWTWTTypeMap[wtType], FMDummyWTIndexTypesMap[indexType], atoi(k.c_str()), atof(loadFactor.c_str()));
		FMDWT->setVerbose(true);
		FMDWT->build(textFileName);
		FMDWT->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m);
        //NegativePatterns *P = new NegativePatterns(textFileName, queriesNum, m);
        /*MaliciousPatterns *P = new MaliciousPatterns(textFileName, m);
        queriesNum = P->getQueriesNum();
        if (queriesNum == 0) exit(1);*/
	unsigned char **patterns = P->getPatterns();
	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMDWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDWT->getIndexSize() / (double)FMDWT->getTextSize();
	cout << "count FMDummyWT-hash-" << wtType << "-" << indexType << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

	delete[] indexCounts;
	delete FMDWT;
	delete P;
        exit(0);
}
