# fmdummy text indexes library

##What is it?
The FMDummy text indexes are fast variants of the FM-index, a well-known compressed full-text index by Ferragina and Manzini (2000). We focus more on search speed than space use. One of the novelties is a rank solution with 1 cache miss in the worst case, which (to our knowledge) was not used earlier elsewhere.

The current version handles only the count query (i.e., returns the number of occurrences of the given pattern).

##Requirements
The FMDummy text indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit operating system

##Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/fmdummy.git
cd fmdummy
make
```

##Usage
To use the fmdummy libriary:
- include "fmdummy/fmdummy.h" to your project
- compile it with "-std=c++11 -O3 -mpopcnt" options and link it with libriaries:
  - fmdummy/libfmdummy.a
  - fmdummy/libs/libaelf64.a

##API
There are several functions you can call on each of the fmdummy text index:
- **building** the index using the text:
```
void build(unsigned char* text, unsigned int textLen);
```
- **saving** the index to file called fileName:
```
void save(char *fileName);
```
- **loading** the index from file called fileName:
```
void load(char *fileName);
```
- **free** memory occupied by index:
```
void free();
```
- get the **index size** in bytes (size in memory):
```
unsigned int getIndexSize();
```
- get the size in bytes of the text used to build the index:
```
unsigned int getTextSize();
```
- get the result of **count** query:
```
unsigned int count(unsigned char *pattern, unsigned int patternLen);
```
- set **verbose** mode:
```
void setVerbose(bool verbose);
```

##FMDummy1
FMDummy1 can be build for up to 16 selected characters from text.

Parameters:
- indexType:
      - "256" (default) - using 256b blocks: 64b of rank data and 192b of text data
      - "512" - using 512b blocks: 64b of rank data and 448b of text data
- selectedChars:
      - up to 16 ordinal character values separated by dots, e.g. "65.67.71.84"
      - "all" (default) - all characters from the text

Constructors:
```
FMDummy1();
FMDummy1(string indexType, string selectedChars);
```

##FMDummy2
Parameters:
- indexType:
      - "256" (default) - using 256b blocks: 64b of rank data and 192b of encoded text data
      - "512" - using 512b blocks: 64b of rank data and 448b of encoded text data
- schema:
      - "SCBO" (default) - using SCBO encoding
      - "CB" - using CB encoding
- bitsPerChars:
      - "4" (default) - using 4 bits to store the encoded symbol
      - "3" - using 3 bits to store the encoded symbol

Constructors:
```
FMDummy2();
FMDummy2(string indexType, string schema, string bitsPerChar);
```

##FMDummy3
FMDummy3 is intended for DNA sequences.

Parameters:
- indexType:
      - "512" (default) - using 512b blocks: 128b of rank data and 384b of text data
      - "1024" - using 1024b blocks: 128b of rank data and 896b of text data

Constructors:
```
FMDummy3();
FMDummy3(string indexType);
```

##FMDummyWT

Parameters:
- wtType:
      - "2" (default) - using wavelet tree for 2-ary Huffman encoded text
      - "4" - using wavelet tree for 4-ary Huffman encoded text
      - "8" - using wavelet tree for 8-ary Huffman encoded text
- indexType:
      - "512" (default) - using 512b blocks: 64b of rank data and 448b of encoded text data
      - "1024" - using 1024b blocks: 64b of rank data and 960b of encoded text data

Constructors:
```
FMDummyWT();
FMDummyWT(string wtType, string indexType);
```

##FMDummy1 usage example
```
#include <iostream>
#include <string>
#include <stdlib.h>
#include "fmdummy/shared/common.h"
#include "fmdummy/shared/patterns.h"
#include "fmdummy/fmdummy.h"

using namespace std;

void fmDummy1(string indexType, string selectedChars, char *textFileName, unsigned int queriesNum, unsigned int patternLen) {

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
	
	double indexSize = (double)FMD1->getIndexSize();
	cout << "Index size: " << indexSize << "B (" << (indexSize / (double)FMD1->getTextSize()) << "n)" << endl << endl;

	Patterns *P = new Patterns(textFileName, queriesNum, patternLen, selectedChars);
	unsigned char **patterns = P->getPatterns();
	
	for (unsigned int i = 0; i < queriesNum; ++i) {
		cout << "Pattern |" << patterns[i] << "| occurs " << FMD1->count(patterns[i], patternLen) << " times." << endl;
	}

	if (text != NULL) delete[] text;
	delete FMD1;
	delete P;
}
```
Using other fmdummy indexes is analogous.
##External resources used in fmdummy project
- Yuta Mori suffix array building (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
- Sebastian Deorowicz
