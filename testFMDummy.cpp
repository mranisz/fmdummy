#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include "shared/common.h"
#include "shared/patterns.h"
#include "shared/timer.h"
#include "fmdummy.h"

using namespace std;

CStopWatch timer;

void getUsage() {
	cout << "Choose index you want to test:" << endl;
	cout << "FMDummy1: ./testFMDummy 1 256c|512c selectedChars fileName q m" << endl;
	cout << "FMDummy2: ./testFMDummy 2 256c|512c SCBO|CB 3|4 fileName q m" << endl;
	cout << "FMDummy3: ./testFMDummy 3 512|1024 fileName q m" << endl;
	cout << "where:" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "q - number of patterns (queries)" << endl;
	cout << "m - pattern length" << endl;
	cout << "selectedChars - up to 15 ordinal character values separated by dots or \"all\" if you want to build index on all characters from the text" << endl << endl;
}

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int queriesNum, unsigned int m);
void fmDummy2(string indexType, string encodedSchema, string bits, char *textFileName, unsigned int queriesNum, unsigned int m);

int main(int argc, char *argv[]) {
	if ((string)argv[1] == "1") {
		if (argc < 7) {
			getUsage();
			exit(1);
		}
		fmDummy1(string(argv[2]), string(argv[3]), argv[4], atoi(argv[5]), atoi(argv[6]));
	}
	else if ((string)argv[1] == "2") {
		if (argc < 8) {
			getUsage();
			exit(1);
		}
		fmDummy2(string(argv[2]), string(argv[3]), string(argv[4]), argv[5], atoi(argv[6]), atoi(argv[7]));
	}
//	else if ((string)argv[1] == "2CB") {
//		if (argc >= 8) fmDummy2CB(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6], atoi(argv[7]));
//		else fmDummy2CB(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6], 4);
//	}
//	else if ((string)argv[1] == "3") {
//		fmDummy3(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6]);
//	}
//	else if ((string)argv[1] == "WT2") {
//		fmDummyWT2(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6], k);
//	}
//	else if ((string)argv[1] == "WT4") {
//		fmDummyWT4(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6], k);
//	}
//	else if ((string)argv[1] == "WT8") {
//		fmDummyWT8(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6], k);
//	}
	else {
		getUsage();
		exit(1);
	}
}

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int queriesNum, unsigned int m) {

	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
	unsigned char **patterns = P->getPatterns();

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy1 *FMD1;
	stringstream ss;
	ss << textFileName << "-" << indexType << "-" << selectedChars << ".fm1_idx";
	string s = ss.str();
	char *indexFileName = (char *)(s.c_str());

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

	Patterns *P = new Patterns(textFileName, queriesNum, m);
	unsigned char **patterns = P->getPatterns();

	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy2 *FMD2;
	stringstream ss;
	ss << textFileName << "-" << indexType << "-" << encodedSchema << "-" << bits << ".fm2_idx";
	string s = ss.str();
	char *indexFileName = (char *)(s.c_str());

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
