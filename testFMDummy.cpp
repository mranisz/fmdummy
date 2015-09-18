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
	cout << "Parameters:" << endl;
	cout << "fmDummy option fileName q m [bits|k|selectedChars]" << endl;
	cout << "where:" << endl;
	cout << "fmDummy - FM Dummy index type [1|2|2CB|3|WT2|WT4|WT8]" << endl;
	cout << "option: [256|512|256c|512c] for type [1|2|2CB] or [512|1024] for type [3|WT2|WT4|WT8] or [512-hash|1024-hash] for type [WT2|WT4|WT8] or [512c|1024c|512c-hash|1024c-hash] for WT2 or [512-SSE2|1024-SSE2] for type [WT4|WT8]" << endl;
	cout << "fileName - name of text file" << endl;
	cout << "q - number of patterns (queries)" << endl;
	cout << "m - pattern length" << endl;
	cout << "bits (optional, only for fmDummy2CB, default bits = 4) - number of bits used to coded a character" << endl;
	cout << "k (optional, only for hash, default bits = 4)" << endl;
	cout << "selectedChars (only for fmDummy1) - up to 15 ordinal character values separated by commas which should be used in alphabet" << endl << endl;
}

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int m, unsigned int queriesNum);

int main(int argc, char *argv[]) {
	if (argc < 6) {
		getUsage();
		return 1;
	}

	if ((string)argv[1] == "1") {
		fmDummy1(string(argv[2]), string(argv[6]), argv[3], atoi(argv[5]), atoi(argv[4]));
	}
//	else if ((string)argv[1] == "2") {
//		fmDummy2(argv[2], argv[3], atoi(argv[5]), atoi(argv[4]), argv[6]);
//	}
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
		return 1;
	}
}

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int m, unsigned int queriesNum) {

	Patterns *P = new Patterns(textFileName, queriesNum, m, selectedChars);
	unsigned char **patterns = P->getPatterns();

	FMDummy1 *FMD1;
	stringstream ss;
	ss << textFileName << "-" << indexType << ".fm1_idx";
	string s = ss.str();
	char *indexFileName = (char *)(s.c_str());

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(indexType, selectedChars);
		FMD1->setVerbose(true);
		FMD1->build(textFileName);
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
	cout << "count FMDummy1_" << indexType << " " << textFileName << " m=" << m << " queries=" << queriesNum << " time=" << timer.getElapsedTime() << " size=" << size << "n";
	resultFile << m << " " << queriesNum << " " << indexType << " " << timer.getElapsedTime() << " " << size;

	unsigned int differences = P->getErrorCountsNumber(indexCounts);
	if (differences > 0) {
		cout << " DIFFERENCES: " << differences;
		resultFile << " DIFFERENCES: " << differences;
	}

	cout << endl;
	resultFile << endl;
	resultFile.close();

	delete[] indexCounts;
	delete FMD1;
	delete P;
}
