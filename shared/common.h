#include <string>

using namespace std;

#ifndef SHARED_COMMON_H_
#define SHARED_COMMON_H_

unsigned long long getFileSize(char* inFileName, int elemSize);
FILE* openFile(char* inFileName, int elemSize, unsigned int &len);
unsigned char *readFileChar(char* inFileName, unsigned int &len, unsigned int addLen);
unsigned int *readFileInt(char* inFileName, unsigned int &len, unsigned int addLen);
unsigned long long *readFileLong(char* inFileName, unsigned int &len, unsigned int addLen);
bool fileExists(char* inFileName);
unsigned int *readSA(char* inFileName, unsigned int &len, unsigned int addLen, bool verbose);
unsigned char *readText(char* inFileName, unsigned int &len, unsigned char eof, bool checkNullChar);
unsigned char *getBwt(char *fileName, unsigned int &len, unsigned char eof, bool verbose);
unsigned int *breakByDelimeter(string seq, char delim, unsigned int &tokensLen);
#endif /* SHARED_COMMON_H_ */
