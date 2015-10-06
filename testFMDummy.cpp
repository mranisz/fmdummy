#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "shared/common.h"
#include "shared/patterns.h"
#include "shared/timer.h"
#include "fmdummy.h"

using namespace std;

CStopWatch timer;

void getUsage(char **argv) {
	cout << "Choose index you want to test:" << endl;
	cout << "FMDummy1: ./" << argv[0] << " 1 256|512 selectedChars fileName q m" << endl;
	cout << "FMDummy2: ./" << argv[0] << " 2 256|512 SCBO|CB 3|4 fileName q m" << endl;
	cout << "FMDummy3: ./" << argv[0] << " 3 512|1024 fileName q m" << endl;
	cout << "FMDummyWT2: ./" << argv[0] << " WT 2 512|1024 fileName q m" << endl;
	cout << "FMDummyWT4: ./" << argv[0] << " WT 4 512|1024 fileName q m" << endl;
	cout << "FMDummyWT8: ./" << argv[0] << " WT 8 512|1024 fileName q m" << endl;
	cout << "where:" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "q - number of patterns (queries)" << endl;
	cout << "m - pattern length" << endl;
	cout << "selectedChars - up to 16 ordinal character values separated by dots or \"all\" if you want to build index on all characters from the text" << endl << endl;
}

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2(string indexType, string encodedSchema, string bits, char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy3(string indexType, char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummyWT(string wtType, string indexType, char *textFileName, unsigned int queriesNum, unsigned int m);

int main(int argc, char *argv[]) {
	if (argc < 3) {
		getUsage(argv);
		return 1;
	}

	if ((string)argv[1] == "1") {
		if (argc < 7) {
			getUsage(argv);
			exit(1);
		}
		fmDummy1(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	}
	else if ((string)argv[1] == "2") {
		if (argc < 8) {
			getUsage(argv);
			exit(1);
		}
		fmDummy2(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	}
	else if ((string)argv[1] == "3") {
		if (argc < 6) {
			getUsage(argv);
			exit(1);
		}
		fmDummy3(string(argv[2]), argv[3], atoi(argv[4]), atoi(argv[5]));
	}
	else if ((string)argv[1] == "WT") {
		if (argc < 7) {
			getUsage(argv);
			exit(1);
		}
		fmDummyWT(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	}
	else {
		getUsage(argv);
		exit(1);
	}
}

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy1 *FMD1;
	string indexFileNameString = string("FMD1-") + (string)textFileName + "-" + indexType + "-" + selectedChars + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(indexType, selectedChars);
		FMD1->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD1->build(text, textLen);
		FMD1->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
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
	cout << "count FMDummy1_" << indexType << " " << selectedChars << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

void fmDummy2(string indexType, string encodedSchema, string bits, char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy2 *FMD2;
	string indexFileNameString = string("FMD2-") + (string)textFileName + "-" + indexType + "-" + encodedSchema + "-" + bits + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD2 = new FMDummy2();
		FMD2->load(indexFileName);
	} else {
		FMD2 = new FMDummy2(indexType, encodedSchema, bits);
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
	cout << "count FMDummy2_" << indexType << " " << encodedSchema << " " << bits << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
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

void fmDummy3(string indexType, char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy3 *FMD3;
	string indexFileNameString = string("FMD3-") + (string)textFileName + "-" + indexType + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMD3 = new FMDummy3();
		FMD3->load(indexFileName);
	} else {
		FMD3 = new FMDummy3(indexType);
		FMD3->setVerbose(true);
		text = readText(textFileName, textLen, 0);
		FMD3->build(text, textLen);
		FMD3->save(indexFileName);
	}

	Patterns *P = new Patterns(textFileName, queriesNum, m, "65.67.71.84");
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

void fmDummyWT(string wtType, string indexType, char *textFileName, unsigned int queriesNum, unsigned int m) {

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummyWT *FMDWT;
	string indexFileNameString = string("FMDWT-") + (string)textFileName + "-" + wtType + "-" + indexType + ".idx";
	char *indexFileName = (char *)indexFileNameString.c_str();

	if (fileExists(indexFileName)) {
		FMDWT = new FMDummyWT();
		FMDWT->load(indexFileName);
	} else {
		FMDWT = new FMDummyWT(wtType, indexType);
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
