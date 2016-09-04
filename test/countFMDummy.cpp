#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <map>
#include "../shared/patterns.h"
#include "../shared/timer.h"
#include "../fmdummy.hpp"

using namespace std;
using namespace shared;
using namespace fmdummy;

ChronoStopWatch timer;

map<string, vector<unsigned char>> FMDummy1SelectedCharsMap = {{"ACGT", {65, 67, 71, 84}}, {"all", {}}};

void fmDummy1_256(string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy1_512(string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy1Hash_256(string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy1Hash_512(string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_256_SCBO_3(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_256_CB_3(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_256_SCBO_4(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_256_CB_4(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_512_SCBO_3(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_512_CB_3(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_512_SCBO_4(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2_512_CB_4(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_256_SCBO_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_256_CB_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_256_SCBO_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_256_CB_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_512_SCBO_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_512_CB_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_512_SCBO_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2Hash_512_CB_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3_512(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3_1024(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT2_512(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT2_1024(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT4_512(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT4_1024(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT8_512(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT8_1024(const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT2Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT2Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT4Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT4Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT8Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyHWT8Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m);

void getUsage(char **argv) {
	cout << "Select index you want to test (count):" << endl;
	cout << "FMDummy1-256: " << argv[0] << " 1-256 all|ACGT fileName patternNum patternLen" << endl;
	cout << "FMDummy1-512: " << argv[0] << " 1-512 all|ACGT fileName patternNum patternLen" << endl;
	cout << "FMDummy2-256-SCBO-3: " << argv[0] << " 2-256-SCBO-3 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-512-SCBO-3: " << argv[0] << " 2-512-SCBO-3 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-256-CB-3: " << argv[0] << " 2-256-CB-3 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-512-CB-3: " << argv[0] << " 2-512-CB-3 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-256-SCBO-4: " << argv[0] << " 2-256-SCBO-4 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-512-SCBO-4: " << argv[0] << " 2-512-SCBO-4 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-256-CB-4: " << argv[0] << " 2-256-CB-4 fileName patternNum patternLen" << endl;
	cout << "FMDummy2-512-CB-4: " << argv[0] << " 2-512-CB-4 fileName patternNum patternLen" << endl;
	cout << "FMDummy3-512: " << argv[0] << " 3-512 fileName patternNum patternLen" << endl;
	cout << "FMDummy3-1024: " << argv[0] << " 3-1024 fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT2-512: " << argv[0] << " HWT2-512 fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT2-1024: " << argv[0] << " HWT2-1024 fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT4-512: " << argv[0] << " HWT4-512 fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT4-1024: " << argv[0] << " HWT4-1024 fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT8-512: " << argv[0] << " HWT8-512 fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT8-1024: " << argv[0] << " HWT8-1024 fileName patternNum patternLen" << endl;
	cout << "FMDummy1-hash-256: " << argv[0] << " 1-hash-256 all|ACGT k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy1-hash-512: " << argv[0] << " 1-hash-512 all|ACGT k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-256-SCBO-3: " << argv[0] << " 2-hash-256-SCBO-3 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-512-SCBO-3: " << argv[0] << " 2-hash-512-SCBO-3 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-256-CB-3: " << argv[0] << " 2-hash-256-CB-3 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-512-CB-3: " << argv[0] << " 2-hash-512-CB-3 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-256-SCBO-4: " << argv[0] << " 2-hash-256-SCBO-4 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-512-SCBO-4: " << argv[0] << " 2-hash-512-SCBO-4 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-256-CB-4: " << argv[0] << " 2-hash-256-CB-4 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy2-hash-512-CB-4: " << argv[0] << " 2-hash-512-CB-4 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy3-hash-512: " << argv[0] << " 3-hash-512 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummy3-hash-1024: " << argv[0] << " 3-hash-1024 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT2-hash-512: " << argv[0] << " HWT2-hash-512 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT2-hash-1024: " << argv[0] << " HWT2-hash-1024 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT4-hash-512: " << argv[0] << " HWT4-hash-512 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT4-hash-1024: " << argv[0] << " HWT4-hash-1024 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT8-hash-512: " << argv[0] << " HWT8-hash-512 k loadFactor fileName patternNum patternLen" << endl;
	cout << "FMDummyHWT8-hash-1024: " << argv[0] << " HWT8-hash-1024 k loadFactor fileName patternNum patternLen" << endl;
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
	if (string(argv[1]) == "1-256") fmDummy1_256(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "1-512") fmDummy1_512(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	if (string(argv[1]) == "2-256-SCBO-3") fmDummy2_256_SCBO_3(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-512-SCBO-3") fmDummy2_512_SCBO_3(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-256-CB-3") fmDummy2_256_CB_3(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-512-CB-3") fmDummy2_512_CB_3(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-256-SCBO-4") fmDummy2_256_SCBO_4(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-512-SCBO-4") fmDummy2_512_SCBO_4(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-256-CB-4") fmDummy2_256_CB_4(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "2-512-CB-4") fmDummy2_512_CB_4(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "3-512") fmDummy3_512(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "3-1024") fmDummy3_1024(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "HWT2-512") fmDummyHWT2_512(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "HWT2-1024") fmDummyHWT2_1024(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "HWT4-512") fmDummyHWT4_512(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "HWT4-1024") fmDummyHWT4_1024(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "HWT8-512") fmDummyHWT8_512(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "HWT8-1024") fmDummyHWT8_1024(argv[2], atoi(argv[3]), atoi(argv[4]));
	if (string(argv[1]) == "1-hash-256") fmDummy1Hash_256(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	if (string(argv[1]) == "1-hash-512") fmDummy1Hash_512(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	if (string(argv[1]) == "2-hash-256-SCBO-3") fmDummy2Hash_256_SCBO_3(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-512-SCBO-3") fmDummy2Hash_512_SCBO_3(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-256-CB-3") fmDummy2Hash_256_CB_3(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-512-CB-3") fmDummy2Hash_512_CB_3(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-256-SCBO-4") fmDummy2Hash_256_SCBO_4(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-512-SCBO-4") fmDummy2Hash_512_SCBO_4(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-256-CB-4") fmDummy2Hash_256_CB_4(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "2-hash-512-CB-4") fmDummy2Hash_512_CB_4(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "3-hash-512") fmDummy3Hash_512(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "3-hash-1024") fmDummy3Hash_1024(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "HWT2-hash-512") fmDummyHWT2Hash_512(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "HWT2-hash-1024") fmDummyHWT2Hash_1024(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "HWT4-hash-512") fmDummyHWT4Hash_512(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "HWT4-hash-1024") fmDummyHWT4Hash_1024(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "HWT8-hash-512") fmDummyHWT8Hash_512(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	if (string(argv[1]) == "HWT8-hash-1024") fmDummyHWT8Hash_1024(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	getUsage(argv);
	exit(1);
}

void fmDummy1_256(string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy1<FMD1_256> *FMD1 = new FMDummy1<FMD1_256>(FMDummy1SelectedCharsMap[selectedChars]);
	string indexFileNameString = "FMD1-" + (string)textFileName + "-256-" + selectedChars + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1->load(indexFileName);
	} else {
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
	cout << "count FMDummy1-256-" << selectedChars << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 " << selectedChars << " " << size << " " << timer.getElapsedTime();

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

void fmDummy1_512(string selectedChars, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy1<FMD1_512> *FMD1 = new FMDummy1<FMD1_512>(FMDummy1SelectedCharsMap[selectedChars]);
	string indexFileNameString = "FMD1-" + (string)textFileName + "-512-" + selectedChars + ".idx";
	const char *indexFileName = indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1->load(indexFileName);
	} else {
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
	cout << "count FMDummy1-512-" << selectedChars << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 " << selectedChars << " " << size << " " << timer.getElapsedTime();

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

void fmDummy1Hash_256(string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy1Hash<FMD1_256> *FMD1 = new FMDummy1Hash<FMD1_256>(FMDummy1SelectedCharsMap[selectedChars], atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD1-hash-" + (string)textFileName + "-256-" + selectedChars + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1->load(indexFileName);
	} else {
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
	cout << "count FMDummy1-hash-256-" << selectedChars << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 " << selectedChars << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy1Hash_512(string selectedChars, string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy1Hash<FMD1_512> *FMD1 = new FMDummy1Hash<FMD1_512>(FMDummy1SelectedCharsMap[selectedChars], atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD1-hash-" + (string)textFileName + "-512-" + selectedChars + "-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1->load(indexFileName);
	} else {
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
	cout << "count FMDummy1-hash-512-" << selectedChars << "-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 " << selectedChars << " " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2_256_SCBO_3(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_3> *FMD2 = new FMDummy2<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_3>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-256-SCBO-3.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-256-SCBO-3 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 SCBO 3 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_512_SCBO_3(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_3> *FMD2 = new FMDummy2<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_3>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-512-SCBO-3.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-512-SCBO-3 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 SCBO 3 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_256_CB_3(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_3> *FMD2 = new FMDummy2<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_3>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-256-CB-3.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-256-CB-3 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 CB 3 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_512_CB_3(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_3> *FMD2 = new FMDummy2<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_3>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-512-CB-3.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-512-CB-3 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 CB 3 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_256_SCBO_4(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_4> *FMD2 = new FMDummy2<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_4>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-256-SCBO-4.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-256-SCBO-4 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 SCBO 4 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_512_SCBO_4(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_4> *FMD2 = new FMDummy2<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_4>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-512-SCBO-4.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-512-SCBO-4 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 SCBO 4 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_256_CB_4(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_4> *FMD2 = new FMDummy2<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_4>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-256-CB-4.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-256-CB-4 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 CB 4 " << size << " " << timer.getElapsedTime();

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

void fmDummy2_512_CB_4(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_4> *FMD2 = new FMDummy2<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_4>();
	string indexFileNameString = "FMD2-" + (string)textFileName + "-512-CB-4.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-512-CB-4 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 CB 4 " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_256_SCBO_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_3> *FMD2 = new FMDummy2Hash<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_3>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-256-SCBO-3-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-256-SCBO-3-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 SCBO 3 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_512_SCBO_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_3> *FMD2 = new FMDummy2Hash<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_3>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-512-SCBO-3-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-512-SCBO-3-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 SCBO 3 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_256_CB_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_3> *FMD2 = new FMDummy2Hash<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_3>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-256-CB-3-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-256-CB-3-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 CB 3 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_512_CB_3(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_3> *FMD2 = new FMDummy2Hash<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_3>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-512-CB-3-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-512-CB-3-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 CB 3 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_256_SCBO_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_4> *FMD2 = new FMDummy2Hash<FMD2_256, FMD2_SCHEMA_SCBO, FMD2_BPC_4>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-256-SCBO-4-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-256-SCBO-4-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 SCBO 4 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_512_SCBO_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_4> *FMD2 = new FMDummy2Hash<FMD2_512, FMD2_SCHEMA_SCBO, FMD2_BPC_4>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-512-SCBO-4-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-512-SCBO-4-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 SCBO 4 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_256_CB_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_4> *FMD2 = new FMDummy2Hash<FMD2_256, FMD2_SCHEMA_CB, FMD2_BPC_4>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-256-CB-4-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-256-CB-4-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 256 CB 4 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy2Hash_512_CB_4(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy2Hash<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_4> *FMD2 = new FMDummy2Hash<FMD2_512, FMD2_SCHEMA_CB, FMD2_BPC_4>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD2-hash-" + (string)textFileName + "-512-CB-4-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2->load(indexFileName);
	} else {
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
	cout << "count FMDummy2-hash-512-CB-4-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 CB 4 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy3_512(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy3<FMD3_512> *FMD3 = new FMDummy3<FMD3_512>();
	string indexFileNameString = "FMD3-" + (string)textFileName + "-512.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3->load(indexFileName);
	} else {
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
	cout << "count FMDummy3-512 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 " << size << " " << timer.getElapsedTime();

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

void fmDummy3_1024(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy3<FMD3_1024> *FMD3 = new FMDummy3<FMD3_1024>();
	string indexFileNameString = "FMD3-" + (string)textFileName + "-1024.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3->load(indexFileName);
	} else {
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
	cout << "count FMDummy3-1024 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 1024 " << size << " " << timer.getElapsedTime();

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

void fmDummy3Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy3Hash<FMD3_512> *FMD3 = new FMDummy3Hash<FMD3_512>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD3-hash-" + (string)textFileName + "-512-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3->load(indexFileName);
	} else {
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
	cout << "count FMDummy3-hash-512-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 512 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummy3Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummy3Hash<FMD3_1024> *FMD3 = new FMDummy3Hash<FMD3_1024>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMD3-hash-" + (string)textFileName + "-1024-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3->load(indexFileName);
	} else {
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
	cout << "count FMDummy3-hash-1024-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 1024 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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

void fmDummyHWT2_512(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWT<FMDHWT_512, WTDummy_2> *FMDHWT = new FMDummyHWT<FMDHWT_512, WTDummy_2>();
	string indexFileNameString = "FMDHWT-" + (string)textFileName + "-2-512.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-2-512 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 2 512 " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
	exit(0);
}

void fmDummyHWT2_1024(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWT<FMDHWT_1024, WTDummy_2> *FMDHWT = new FMDummyHWT<FMDHWT_1024, WTDummy_2>();
	string indexFileNameString = "FMDHWT-" + (string)textFileName + "-2-1024.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-2-1024 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 2 1024 " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
	exit(0);
}

void fmDummyHWT4_512(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWT<FMDHWT_512, WTDummy_4> *FMDHWT = new FMDummyHWT<FMDHWT_512, WTDummy_4>();
	string indexFileNameString = "FMDHWT-" + (string)textFileName + "-4-512.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-4-512 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 4 512 " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
	exit(0);
}

void fmDummyHWT4_1024(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWT<FMDHWT_1024, WTDummy_4> *FMDHWT = new FMDummyHWT<FMDHWT_1024, WTDummy_4>();
	string indexFileNameString = "FMDHWT-" + (string)textFileName + "-4-1024.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-4-1024 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 4 1024 " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
	exit(0);
}

void fmDummyHWT8_512(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWT<FMDHWT_512, WTDummy_8> *FMDHWT = new FMDummyHWT<FMDHWT_512, WTDummy_8>();
	string indexFileNameString = "FMDHWT-" + (string)textFileName + "-8-512.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-8-512 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 8 512 " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
	exit(0);
}

void fmDummyHWT8_1024(const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWT<FMDHWT_1024, WTDummy_8> *FMDHWT = new FMDummyHWT<FMDHWT_1024, WTDummy_8>();
	string indexFileNameString = "FMDHWT-" + (string)textFileName + "-8-1024.idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-8-1024 " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 8 1024 " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
	exit(0);
}

void fmDummyHWT2Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWTHash<FMDHWT_512, WTDummy_2> *FMDHWT = new FMDummyHWTHash<FMDHWT_512, WTDummy_2>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMDHWT-hash-" + (string)textFileName + "-2-512-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-hash-2-512-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 2 512 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
    exit(0);
}

void fmDummyHWT2Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWTHash<FMDHWT_1024, WTDummy_2> *FMDHWT = new FMDummyHWTHash<FMDHWT_1024, WTDummy_2>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMDHWT-hash-" + (string)textFileName + "-2-1024-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-hash-2-1024-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 2 1024 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
    exit(0);
}

void fmDummyHWT4Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWTHash<FMDHWT_512, WTDummy_4> *FMDHWT = new FMDummyHWTHash<FMDHWT_512, WTDummy_4>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMDHWT-hash-" + (string)textFileName + "-4-512-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-hash-4-512-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 4 512 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
    exit(0);
}

void fmDummyHWT4Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWTHash<FMDHWT_1024, WTDummy_4> *FMDHWT = new FMDummyHWTHash<FMDHWT_1024, WTDummy_4>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMDHWT-hash-" + (string)textFileName + "-4-1024-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-hash-4-1024-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 4 1024 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
    exit(0);
}

void fmDummyHWT8Hash_512(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWTHash<FMDHWT_512, WTDummy_8> *FMDHWT = new FMDummyHWTHash<FMDHWT_512, WTDummy_8>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMDHWT-hash-" + (string)textFileName + "-8-512-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-hash-8-512-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 8 512 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
    exit(0);
}

void fmDummyHWT8Hash_1024(string k, string loadFactor, const char *textFileName, unsigned int queriesNum, unsigned int m) {
	FMDummyHWTHash<FMDHWT_1024, WTDummy_8> *FMDHWT = new FMDummyHWTHash<FMDHWT_1024, WTDummy_8>(atoi(k.c_str()), atof(loadFactor.c_str()));
	string indexFileNameString = "FMDHWT-hash-" + (string)textFileName + "-8-1024-" +  k + "-" + loadFactor + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDHWT->load(indexFileName);
	} else {
		FMDHWT->setVerbose(true);
		FMDHWT->build(textFileName);
		FMDHWT->save(indexFileName);
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
		indexCounts[i] = FMDHWT->count(patterns[i], m);
	}
	timer.stopTimer();

	string resultFileName = "results/fmdummy/" + string(textFileName) + "_count_FMDummyHWT-hash.txt";
	fstream resultFile(resultFileName.c_str(), ios::out | ios::binary | ios::app);
	double size = (double)FMDHWT->getIndexSize() / (double)FMDHWT->getTextSize();
	cout << "count FMDummyHWT-hash-8-1024-" << k << "-" << loadFactor << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
	resultFile << m << " " << queriesNum << " 8 1024 " << k << " " << loadFactor << " " << size << " " << timer.getElapsedTime();

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
	delete FMDHWT;
	delete P;
    exit(0);
}