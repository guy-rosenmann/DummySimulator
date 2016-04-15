﻿#ifndef __ALGORITHMCONTAINER__H_
#define __ALGORITHMCONTAINER__H_

#include "Direction.h"
#include "AlgorithmRegistrar.h"

class AlgorithmContainer
{
	AbstractAlgorithm*		_algorithm;

	bool					_isValid;
	string					_fileName;
	string					_algoName;
	string					_errorLine;

public:

	AlgorithmContainer() = delete;
	AlgorithmContainer(const char* algorithmPath_);
	// TODO: remove the name
	AlgorithmContainer(AbstractAlgorithm* algorithm_) : _algorithm(algorithm_), _algoName("201445681_A_"){}

	bool isValid() const { return _isValid; }
	string getFileName() const { return _fileName; }
	string GetAlgorithmName() const { return _algoName; }
	string getErrorLine() const { return _errorLine; }
	AbstractAlgorithm* getAlgorithm() const { return _algorithm; }

};


#endif //__ALGORITHMCONTAINER__H_