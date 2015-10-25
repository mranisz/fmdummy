#include <string.h>

using namespace std;

#ifndef SHARED_COMMON_H_
#define SHARED_COMMON_H_

unsigned long long getFileSize(char *inFileName, int elemSize);
FILE *openFile(char *inFileName, int elemSize, unsigned int &len);
unsigned char *readFileChar(char *inFileName, unsigned int &len, unsigned int addLen);
unsigned int *readFileInt(char *inFileName, unsigned int &len, unsigned int addLen);
unsigned long long *readFileLong(char *inFileName, unsigned int &len, unsigned int addLen);
bool fileExists(char *inFileName);
unsigned char *readText(char *inFileName, unsigned int &textLen, unsigned char eof);
void checkNullChar(unsigned char *text, unsigned int textLen);
unsigned int *getSA(unsigned char *text, unsigned int textLen, unsigned int &saLen, unsigned int addLen, bool verbose);
unsigned char *getBWT(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen, unsigned int &bwtLen, bool verbose);
unsigned char *getBWT(unsigned char *text, unsigned int textLen, unsigned int &bwtLen, bool verbose);
unsigned int getUniqueSuffixNum(unsigned char *text, unsigned int textLen, unsigned int *sa, unsigned int saLen, unsigned int k);
unsigned int getHashValue(unsigned char* str);
void fillArrayC(unsigned char *text, unsigned int textLen, unsigned int* C, bool verbose);
unsigned int *breakByDelimeter(string seq, char delim, unsigned int &tokensLen);
void binarySearch(unsigned int *sa, unsigned char *text, unsigned int lStart, unsigned int rStart, unsigned char *pattern, int patternLength, unsigned int &beg, unsigned int &end);
void fillLUT1(unsigned int lut1[256][2], unsigned char *text, unsigned int *sa, unsigned int saLen);
void fillLUT2(unsigned int lut2[256][256][2], unsigned char *text, unsigned int *sa, unsigned int saLen);
void encode(unsigned char *pattern, unsigned int patternLen, unsigned char *encodedChars, unsigned int *encodedCharsLen, unsigned int maxEncodedCharsLen, unsigned char *encodedPattern, unsigned int &encodedPatternLen);

#endif /* SHARED_COMMON_H_ */
