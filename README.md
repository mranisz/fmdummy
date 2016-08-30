# FMDummy text indexes library

##What is it?
The FMDummy text indexes are fast variants of the FM-index, a well-known compressed full-text index by Ferragina and Manzini (2000). We focus more on search speed than space use. One of the novelties is a rank solution with 1 cache miss in the worst case, which (to our knowledge) was not used earlier elsewhere.

The current version handles only the count query (i.e., returns the number of occurrences of the given pattern).

##Requirements
The FMDummy text indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit operating system
- text size is limited to (FMDummy2 limitations are the worst cases for incompressible text, usually they are not so strong):
    - 4GB for FMDummy1, FMDummy3 and FMDummyHWT
    - 0.8GB for FMDummy2 with FMDummy2Schema::SCHEMA_SCBO and FMDummy2BPC::BITS_3
    - 1.2GB for FMDummy2 with FMDummy2Schema::SCHEMA_SCBO and FMDummy2BPC::BITS_4
    - 1.1GB for FMDummy2 with FMDummy2Schema::SCHEMA_CB and FMDummy2BPC::BITS_3
    - 1.5GB for FMDummy2 with FMDummy2Schema::SCHEMA_CB and FMDummy2BPC::BITS_4

##Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/fmdummy.git
cd fmdummy
make
```

##Usage
To use the FMDummy library:
- include "fmdummy/fmdummy.hpp" to your project
- compile it with "-std=c++11 -O3 -mpopcnt" options and link it with libraries:
  - fmdummy/libfmdummy.a
  - fmdummy/libs/libaelf64.a (linux) or fmdummy/libs/libacof64.lib (windows)
- use "fmdummy" and "shared" namespaces

##API
There are several functions you can call on each of the FMDummy text index:
- **build** the index using text file called textFileName:
```
void build(const char *textFileName);
```
- **save** the index to file called fileName:
```
void save(const char *fileName);
```
- **load** the index from file called fileName:
```
void load(const char *fileName);
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

##FMDummy1\<FMDummy1Type T\>
FMDummy1 can be built for up to 16 selected characters from text.

Parameters:
- T:
      - FMDummy1Type::FMD1_256 - using 256b blocks: 64b of rank data and 192b of text data
      - FMDummy1Type::FMD1_512 - using 512b blocks: 64b of rank data and 448b of text data
- selectedChars:
      - up to 16 ordinal character values, e.g. {'A','C','G','T'}
      - {} (default) - all characters from the text

Constructors:
```
FMDummy1<FMDummy1Type T>();
FMDummy1<FMDummy1Type T>(vector<unsigned char> selectedChars);
```

##FMDummy1Hash\<FMDummy1Type T\>
FMDummy1Hash is FMDummy1 with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- T:
      - FMDummy1Type::FMD1_256 - using 256b blocks: 64b of rank data and 192b of text data
      - FMDummy1Type::FMD1_512 - using 512b blocks: 64b of rank data and 448b of text data
- selectedChars:
      - up to 16 ordinal character values, e.g. {'A','C','G','T'}
      - {} (default) - all characters from the text
- k - length of prefixes of suffixes from suffix array
- loadFactor - hash table load factor

Limitations: 
- pattern length ≥ k (patterns shorter than k are handled by standard variant of FMDummy1 index)
- k ≥ 2
- 0.0 < loadFactor < 1.0

Constructors:
```
FMDummy1Hash<FMDummy1Type T>(unsigned int k, double loadFactor);
FMDummy1Hash<FMDummy1Type T>(vector<unsigned char> selectedChars, unsigned int k, double loadFactor);
```

##FMDummy2\<FMDummy2Type T, FMDummy2Schema S, FMDummy2BPC BPC\>

Parameters:
- T:
      - FMDummy2Type::FMD2_256 - using 256b blocks: 64b of rank data and 192b of encoded text data
      - FMDummy2Type::FMD2_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
- S:
      - FMDummy2Schema::SCHEMA_SCBO - using SCBO encoding (A. Fariña, G. Navarro, J. Paramá. Boosting text compression with word-based statistical encoding. The Computer Journal, 55(1):111–131, 2012)
      - FMDummy2Schema::SCHEMA_CB - using CB encoding (Sz. Grabowski, M. Raniszewski, S. Deorowicz. FM-index for dummies. arXiv:1506.04896, 2015)
- BPC:
      - FMDummy2BPC::BITS_4 - using 4 bits to store the encoded symbol
      - FMDummy2BPC::BITS_3 - using 3 bits to store the encoded symbol

Constructors:
```
FMDummy2<FMDummy2Type T, FMDummy2Schema S, FMDummy2BPC BPC>();
```

##FMDummy2Hash\<FMDummy2Type T, FMDummy2Schema S, FMDummy2BPC BPC\>
FMDummy2Hash is FMDummy2 with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- T:
      - FMDummy2Type::FMD2_256 - using 256b blocks: 64b of rank data and 192b of encoded text data
      - FMDummy2Type::FMD2_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
- S:
      - FMDummy2Schema::SCHEMA_SCBO - using SCBO encoding (A. Fariña, G. Navarro, J. Paramá. Boosting text compression with word-based statistical encoding. The Computer Journal, 55(1):111–131, 2012)
      - FMDummy2Schema::SCHEMA_CB - using CB encoding (Sz. Grabowski, M. Raniszewski, S. Deorowicz. FM-index for dummies. arXiv:1506.04896, 2015)
- BPC:
      - FMDummy2BPC::BITS_4 - using 4 bits to store the encoded symbol
      - FMDummy2BPC::BITS_3 - using 3 bits to store the encoded symbol
- k - length of prefixes of suffixes from suffix array
- loadFactor - hash table load factor

Limitations: 
- pattern length ≥ k (patterns shorter than k are handled by standard variant of FMDummy2 index)
- k ≥ 2
- 0.0 < loadFactor < 1.0

Constructors:
```
FMDummy2Hash<FMDummy2Type T, FMDummy2Schema S, FMDummy2BPC BPC>(unsigned int k, double loadFactor);
```

##FMDummy3\<FMDummy3Type T\>
FMDummy3 is intended for DNA sequences (it searches only for patterns consisting of the symbols A, C, G, T).

Parameters:
- T:
      - FMDummy3Type::FMD3_512 - using 512b blocks: 128b of rank data and 384b of text data
      - FMDummy3Type::FMD3_1024 - using 1024b blocks: 128b of rank data and 896b of text data

Constructors:
```
FMDummy3<FMDummy3Type T>();
```

##FMDummy3Hash\<FMDummy3Type T\>
FMDummy3Hash is FMDummy3 with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- T:
      - FMDummy3Type::FMD3_512 - using 512b blocks: 128b of rank data and 384b of text data
      - FMDummy3Type::FMD3_1024 - using 1024b blocks: 128b of rank data and 896b of text data
- k - length of prefixes of suffixes from suffix array
- loadFactor - hash table load factor

Limitations: 
- pattern length ≥ k (patterns shorter than k are handled by standard variant of FMDummy3 index)
- k ≥ 2
- 0.0 < loadFactor < 1.0

Constructors:
```
FMDummy3Hash<FMDummy3Type T>(unsigned int k, double loadFactor);
```

##FMDummyHWT\<FMDummyHWTType T, WTType W\>

Parameters:
- T:
      - FMDummyHWTType::FMDHWT_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
      - FMDummyHWTType::FMDHWT_1024 - using 1024b blocks: 64b of rank data and 960b of encoded text data
- W:
      - WTType::WT2 - using binary Huffman-shaped wavelet tree
      - WTType::WT4 - using 4-ary Huffman-shaped wavelet tree
      - WTType::WT8 - using 8-ary Huffman-shaped wavelet tree

Constructors:
```
FMDummyHWT<FMDummyHWTType T, WTType W>();
```

##FMDummyHWTHash\<FMDummyHWTType T, WTType W\>
FMDummyHWTHash is FMDummyHWT with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- T:
      - FMDummyHWTType::FMDHWT_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
      - FMDummyHWTType::FMDHWT_1024 - using 1024b blocks: 64b of rank data and 960b of encoded text data
- W:
      - WTType::WT2 - using binary Huffman-shaped wavelet tree
      - WTType::WT4 - using 4-ary Huffman-shaped wavelet tree
      - WTType::WT8 - using 8-ary Huffman-shaped wavelet tree
- k - length of prefixes of suffixes from suffix array
- loadFactor - hash table load factor

Limitations: 
- pattern length ≥ k (patterns shorter than k are handled by standard variant of FMDummyHWT index)
- k ≥ 2
- 0.0 < loadFactor < 1.0

Constructors:
```
FMDummyHWTHash<FMDummyHWTType T, WTType W>(unsigned int k, double loadFactor);
```

##FMDummy1 usage example
```
#include <iostream>
#include <stdlib.h>
#include "fmdummy/shared/patterns.h"
#include "fmdummy/fmdummy.hpp"

using namespace std;
using namespace shared;
using namespace fmdummy;

int main(int argc, char *argv[]) {

	unsigned int queriesNum = 1000000;
	unsigned int patternLen = 20;
	vector<unsigned char> selectedChars = {'A', 'C', 'G', 'T'};
	FMDummy1<FMDummy1Type::FMD1_256> *FMD1 = new FMDummy1<FMDummy1Type::FMD1_256>(selectedChars);
	const char *textFileName = "dna";
	const char *indexFileName = "dna-fm1.idx";

	if (fileExists(indexFileName)) {
		FMD1->load(indexFileName);
	} else {
		FMD1->setVerbose(true);
		FMD1->build(textFileName);
		FMD1->save(indexFileName);
	}

	double indexSize = (double)FMD1->getIndexSize();
	cout << "Index size: " << indexSize << "B (" << (indexSize / (double)FMD1->getTextSize()) << "n)" << endl << endl;

	Patterns *P = new Patterns(textFileName, queriesNum, patternLen, selectedChars);
	unsigned char **patterns = P->getPatterns();

	for (unsigned int i = 0; i < queriesNum; ++i) {
		cout << "Pattern |" << patterns[i] << "| occurs " << FMD1->count(patterns[i], patternLen) << " times." << endl;
	}

	delete FMD1;
	delete P;
}
```
Using other FMDummy indexes is analogous.

##External resources used in FMDummy project
- Suffix array building by Yuta Mori (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)
- A very fast hash function by Yann Collet (xxHash)

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
- Sebastian Deorowicz
