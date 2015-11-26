# FMDummy text indexes library

##What is it?
The FMDummy text indexes are fast variants of the FM-index, a well-known compressed full-text index by Ferragina and Manzini (2000). We focus more on search speed than space use. One of the novelties is a rank solution with 1 cache miss in the worst case, which (to our knowledge) was not used earlier elsewhere.

The current version handles only the count query (i.e., returns the number of occurrences of the given pattern).

##Requirements
The FMDummy text indexes require:
- C++11 ready compiler such as g++ version 4.7 or higher
- a 64-bit operating system
- text size is limited to (FMDummy2 limitations are the worst cases for incompressible text, usually they are not so strong):
    - 4GB for FMDummy1, FMDummy3 and FMDummyWT
    - 0.8GB for FMDummy2 with SCBO schema and 3 bitsPerChar
    - 1.2GB for FMDummy2 with SCBO schema and 4 bitsPerChar
    - 1.1GB for FMDummy2 with CB schema and 3 bitsPerChar
    - 1.5GB for FMDummy2 with CB schema and 4 bitsPerChar


##Installation
To download and build the library use the following commands:
```
git clone https://github.com/mranisz/fmdummy.git
cd fmdummy
make
```

##Usage
To use the FMDummy library:
- include "fmdummy/fmdummy.h" to your project
- compile it with "-std=c++11 -O3 -mpopcnt" options and link it with libraries:
  - fmdummy/libfmdummy.a
  - fmdummy/libs/libaelf64.a (linux) or fmdummy/libs/libacof64.lib (windows)
- use "fmdummy" namespace

##API
There are several functions you can call on each of the FMDummy text index:
- **build** the index using the text:
```
void build(unsigned char* text, unsigned int textLen);
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

##FMDummy1
FMDummy1 can be built for up to 16 selected characters from text.

Parameters:
- indexType:
      - FMDummy1::TYPE_256 (default) - using 256b blocks: 64b of rank data and 192b of text data
      - FMDummy1::TYPE_512 - using 512b blocks: 64b of rank data and 448b of text data
- selectedChars:
      - up to 16 ordinal character values, e.g. {'A','C','G','T'}
      - {} (default) - all characters from the text

Constructors:
```
FMDummy1();
FMDummy1(FMDummy1::IndexType indexType, vector<unsigned char> selectedChars);
```

##FMDummy1-hash
FMDummy1-hash is FMDummy1 with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- indexType:
      - FMDummy1::TYPE_256 - using 256b blocks: 64b of rank data and 192b of text data
      - FMDummy1::TYPE_512 - using 512b blocks: 64b of rank data and 448b of text data
- selectedChars:
      - up to 16 ordinal character values, e.g. {'A','C','G','T'}
      - {} (default) - all characters from the text
- k - length of prefixes of suffixes from suffix array (k ≥ 2)
- loadFactor - hash table load factor (0.0 < loadFactor < 1.0)

Limitations: 
- pattern length >= k (patterns shorter than k are handled by standard variant of FMDummy1 index)

Constructors:
```
FMDummy1(FMDummy1::IndexType indexType, vector<unsigned char> selectedChars, unsigned int k, double loadFactor);
```

##FMDummy2
Parameters:
- indexType:
      - FMDummy2::TYPE_256 (default) - using 256b blocks: 64b of rank data and 192b of encoded text data
      - FMDummy2::TYPE_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
- schema:
      - FMDummy2::SCHEMA_SCBO (default) - using SCBO encoding (A. Fariña, G. Navarro, J. Paramá. Boosting text compression with word-based statistical encoding. The Computer Journal, 55(1):111–131, 2012)
      - FMDummy2::SCHEMA_CB - using CB encoding
- bitsPerChars:
      - FMDummy2::BITS_4 (default) - using 4 bits to store the encoded symbol
      - FMDummy2::BITS_3 - using 3 bits to store the encoded symbol

Constructors:
```
FMDummy2();
FMDummy2(FMDummy2::IndexType indexType, FMDummy2::Schema schema, FMDummy2::BitsPerChar bitsPerChar);
```

##FMDummy2-hash
FMDummy2-hash is FMDummy2 with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- indexType:
      - FMDummy2::TYPE_256 - using 256b blocks: 64b of rank data and 192b of encoded text data
      - FMDummy2::TYPE_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
- schema:
      - FMDummy2::SCHEMA_SCBO - using SCBO encoding (A. Fariña, G. Navarro, J. Paramá. Boosting text compression with word-based statistical encoding. The Computer Journal, 55(1):111–131, 2012)
      - FMDummy2::SCHEMA_CB - using CB encoding (Sz. Grabowski, M. Raniszewski, S. Deorowicz. FM-index for dummies. arXiv:1506.04896, 2015)
- bitsPerChars:
      - FMDummy2::BITS_4 (default) - using 4 bits to store the encoded symbol
      - FMDummy2::BITS_3 - using 3 bits to store the encoded symbol
- k - length of prefixes of suffixes from suffix array (k ≥ 2)
- loadFactor - hash table load factor (0.0 < loadFactor < 1.0)

Limitations: 
- pattern length >= k (patterns shorter than k are handled by standard variant of FMDummy2 index)

Constructors:
```
FMDummy2(FMDummy2::IndexType indexType, FMDummy2::Schema schema, FMDummy2::BitsPerChar bitsPerChar, unsigned int k, double loadFactor);
```

##FMDummy3
FMDummy3 is intended for DNA sequences (it searches only for patterns consisting of the symbols A, C, G, T).

Parameters:
- indexType:
      - FMDummy3::TYPE_512 (default) - using 512b blocks: 128b of rank data and 384b of text data
      - FMDummy3::TYPE_1024 - using 1024b blocks: 128b of rank data and 896b of text data

Constructors:
```
FMDummy3();
FMDummy3(FMDummy3::IndexType indexType);
```

##FMDummy3-hash
FMDummy3-hash is FMDummy3 with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- indexType:
      - FMDummy3::TYPE_512 - using 512b blocks: 128b of rank data and 384b of text data
      - FMDummy3::TYPE_1024 - using 1024b blocks: 128b of rank data and 896b of text data
- k - length of prefixes of suffixes from suffix array (k ≥ 2)
- loadFactor - hash table load factor (0.0 < loadFactor < 1.0)

Limitations: 
- pattern length >= k (patterns shorter than k are handled by standard variant of FMDummy3 index)

Constructors:
```
FMDummy3(FMDummy3::IndexType indexType, unsigned int k, double loadFactor);
```

##FMDummyWT
Parameters:
- wtType:
      - FMDummyWT::TYPE_WT2 (default) - using wavelet tree for 2-ary Huffman encoded text
      - FMDummyWT::TYPE_WT4 - using wavelet tree for 4-ary Huffman encoded text
      - FMDummyWT::TYPE_WT8 - using wavelet tree for 8-ary Huffman encoded text
- indexType:
      - FMDummyWT::TYPE_512 (default) - using 512b blocks: 64b of rank data and 448b of encoded text data
      - FMDummyWT::TYPE_1024 - using 1024b blocks: 64b of rank data and 960b of encoded text data

Constructors:
```
FMDummyWT();
FMDummyWT(FMDummyWT::WTType wtType, FMDummyWT::IndexType indexType);
```

##FMDummyWT-hash
FMDummyWT-hash is FMDummyWT with hashed k-symbol prefixes of suffixes from suffix array to speed up searches (k ≥ 2). This variant is particularly efficient in speed for short patterns (not much longer than k).

Parameters:
- wtType:
      - FMDummyWT::TYPE_WT2 - using wavelet tree for 2-ary Huffman encoded text
      - FMDummyWT::TYPE_WT4 - using wavelet tree for 4-ary Huffman encoded text
      - FMDummyWT::TYPE_WT8 - using wavelet tree for 8-ary Huffman encoded text
- indexType:
      - FMDummyWT::TYPE_512 - using 512b blocks: 64b of rank data and 448b of encoded text data
      - FMDummyWT::TYPE_1024 - using 1024b blocks: 64b of rank data and 960b of encoded text data
- k - length of prefixes of suffixes from suffix array (k ≥ 2)
- loadFactor - hash table load factor (0.0 < loadFactor < 1.0)

Limitations: 
- pattern length >= k (patterns shorter than k are handled by standard variant of FMDummyWT index)

Constructors:
```
FMDummyWT(FMDummyWT::WTType wtType, FMDummyWT::IndexType indexType, unsigned int k, double loadFactor);
```

##FMDummy1 usage example
```
#include <iostream>
#include <stdlib.h>
#include "fmdummy/shared/common.h"
#include "fmdummy/shared/patterns.h"
#include "fmdummy/fmdummy.h"

using namespace std;
using namespace fmdummy;

int main(int argc, char *argv[]) {

	unsigned int queriesNum = 1000000;
	unsigned int patternLen = 20;
	unsigned char* text = NULL;
	unsigned int textLen;
	FMDummy1 *FMD1;
	const char *textFileName = "dna";
	const char *indexFileName = "dna-fm1.idx";
	vector<unsigned char> selectedChars = {'A', 'C', 'G', 'T'};

	if (fileExists(indexFileName)) {
		FMD1 = new FMDummy1();
		FMD1->load(indexFileName);
	} else {
		FMD1 = new FMDummy1(FMDummy1::TYPE_256, selectedChars);
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
Using other FMDummy indexes is analogous.

##External resources used in FMDummy project
- Suffix array building by Yuta Mori (sais)
- A multi-platform library of highly optimized functions for C and C++ by Agner Fog (asmlib)
- A very fast hash function by Yann Collet (xxHash)

##Authors
- Szymon Grabowski
- [Marcin Raniszewski](https://github.com/mranisz)
- Sebastian Deorowicz
