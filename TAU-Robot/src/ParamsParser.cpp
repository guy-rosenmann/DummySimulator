#include "ParamsParser.h"

#include <iostream>
#include <cstring>

using namespace std;


const char* const ParamsParser::_options[] = {
	"-config",
	"-house_path",
	"-algorithm_path"
};


ParamsParser::ParamsParser(int argc, char* argv[])
{
	bool printUsage = false;

	for (int i = 1; i < argc; ++i)
	{
		unsigned int j, optionsSize = sizeof(_options) / sizeof(*_options);
		for (j = 0; j < optionsSize; ++j)
		{
			if (!strcmp(argv[i], _options[j]) && (i+1 < argc))
			{
				_params[argv[i]] = argv[i + 1];
				++i;
				break;
			}
		}
		if (j == optionsSize)
		{
#ifdef _DEBUG_
			cout << "[WARN] Incompatible argument: " << argv[i] << endl;
#endif
			printUsage = true;
		}
	}

	if (printUsage)
	{
		cout << "USAGE:\tsimulator [-config <path>] [-house_path <path>] [-algorithm_path <path>]" << endl;
		cout << "\t- config:\tconfiguration file location path" << endl;
		cout << "\t- house_path:\thouse dir path" << endl;
		cout << "\t- algorithm_path:\talgorithm dir path" << endl;
		//cout << "\t- house_path:\thouse files directory path" << endl; // for Ex1 it is a house file path
		cout << "\tDefault paths = current working directory" << endl;
		cout << endl;
	}
}


bool ParamsParser::has_key(const string& key) const
{
	map<string, string>::const_iterator it = _params.find(key);
	return (it != _params.end());
}


const char* ParamsParser::operator[](const string& key) const
{
	if (this->has_key(key))
	{
		return _params.at(key).c_str();
	}
	return NULL;
}


const char* ParamsParser::operator[](const char* key) const
{
	string strKey = key;
	return this->operator[](strKey);
}