# fmdummy text indexes library

##What is it?
The FMDummy text indexes are ...

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
- compile it with libriaries:
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
- get the **index size** (size in memory):
```
unsigned int getIndexSize();
```
- get the size of the text used to build the index:
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
      - up to 16 ordinal character values separated by dots
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
  //...
	FMDummy1 *FMD1;
  //...
  
	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(indexType, selectedChars);
		FMD1->setVerbose(true);
		FMD1->build(text, textLen);
		FMD1->save(indexFileName);
	}

	unsigned int *indexCounts = new unsigned int[queriesNum];

	timer.startTimer();
	for (unsigned int i = 0; i < queriesNum; ++i) {
		indexCounts[i] = FMD1->count(patterns[i], patternLen);
	}
	timer.stopTimer();

	double size = (double)FMD1->getIndexSize() / (double)FMD1->getTextSize();
	cout << "count FMDummy1_" << indexType << " " << selectedChars << " " << textFileName << " m=" << m << " queries=" << queriesNum << " size=" << size << "n time=" << timer.getElapsedTime() << endl;
  
  //..
	delete[] indexCounts;
	delete FMD1;
}
```
Using other fmdummy indexes is analogous.
##External Resources used in fmdummy project
- Yuta Mori suffix array building (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
- Sebastian Deorowicz
