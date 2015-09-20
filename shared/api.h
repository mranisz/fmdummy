#ifndef API_H_
#define API_H_

class I_Api {
protected:
	bool verbose = false;
public:
	virtual void build(unsigned char* text, unsigned int textLen) = 0;
	virtual void save(char *fileName) = 0;
	virtual void load(char *fileName) = 0;
	virtual void free() = 0;
	virtual unsigned int getIndexSize() = 0;
	virtual unsigned int getTextSize() = 0;

	virtual unsigned int count(unsigned char *pattern, unsigned int patternLen) = 0;
	virtual unsigned int *locate(unsigned char *pattern, unsigned int patternLen) = 0;
	virtual ~I_Api() {};

	virtual void setVerbose(bool verbose) {
		this->verbose = verbose;
	}

};

#endif /* API_H_ */

