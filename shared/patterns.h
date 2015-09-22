#ifndef SHARED_PATTERNS_H_
#define SHARED_PATTERNS_H_

class Patterns {
private:
	char *textFileName;
	unsigned int queriesNum;
	unsigned int m;
	string selectedChars;
	unsigned int ordCharsLen = 0;
	unsigned int *ordChars = NULL;
	unsigned char **patterns;
	unsigned int *counts = NULL;

	void initialize();
	void freeMemory();
	void initializePatterns();
	void initializeSACounts();
	unsigned int getSACount(unsigned int *sa, unsigned char *text, unsigned int saLen, unsigned char *pattern, int patternLength);
	void binarySearch(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
	void setSelectedChars(string selectedChars);

public:
	Patterns(char *textFileName, unsigned int queriesNum, unsigned int m, string selectedChars = "all") {
		this->textFileName = textFileName;
		this->queriesNum = queriesNum;
		this->m = m;
		this->setSelectedChars(selectedChars);
		this->initialize();
	}
	~Patterns() {
		this->freeMemory();
	}
	unsigned char **getPatterns();
	unsigned int *getSACounts();
	unsigned int getErrorCountsNumber(unsigned int *countsToCheck);
};

#endif /* SHARED_PATTERNS_H_ */
