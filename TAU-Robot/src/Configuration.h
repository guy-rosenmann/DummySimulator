#ifndef __CONFIG_PARSER__H_
#define __CONFIG_PARSER__H_

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;





class Configuration
{
	static string configFileName;
	
	map<string, int>	_params;
	bool				_successful; // did last loading of config was OK

public:
	Configuration() : _successful(true) { this->loadDefaultConfig(); }
	Configuration(const string& iniPath_);
	Configuration(const Configuration& other) : _params(other._params), _successful(true) {}

	bool isReady() { return _successful; }
	void loadDefaultConfig();
	bool loadFromFile(const string& iniPath_);
	void writeConfigFile(const string& iniPath_) const;

	map<string, int> getParams() const { return _params; }

	int& operator[](const string& key) { return _params[key]; }
	int& operator[](const char* key) { return _params[key]; }
	const int operator[](const string& key) const { return _params.at(key); }
	const int operator[](const char* key) const { return _params.at(key); }
	Configuration& operator=(const Configuration& other) { _params = other._params; return *this; }

	string toString() const;
	void print(ostream& out = cout) const { out << this->toString(); }
	friend ostream& operator<<(ostream& out, const Configuration& p) { p.print(out); return out; }

private:

	static std::vector<std::string> split(const std::string& s, char delim);
	static std::string trim(std::string& str);

	void processLine(const string& line);
	void resetConfiguration();
	
};


#endif //__CONFIG_PARSER__H_