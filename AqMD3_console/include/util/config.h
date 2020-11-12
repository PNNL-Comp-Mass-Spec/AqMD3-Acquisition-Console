#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <experimental/filesystem>
#include <map>
#include <sstream>
#include <vector>
#include <iostream>
using std::cerr;

class Config
{
private:
	std::string config_filename;
	std::map<std::string, std::string> config_map;

public:
	Config(std::string filename)
	: config_filename(filename)
	{}

	bool exists() {
		return std::experimental::filesystem::exists(config_filename);
	}
	
	void read() {
		if (!exists())
			return;

		std::ifstream config(config_filename);

		std::string line;
		while (std::getline(config, line))
		{
			// ignore empty lines
			if (line.length() == 0)
				continue;

			// ignore comment lines
			if (line.rfind("#", 0) == 0)
				continue;

			std::istringstream sline(line);
			std::vector<std::string> strings;
			std::string tok;
		
			while (getline(sline, tok, '=')) {
				strings.push_back(tok);
			}

			if (strings.size() != 2)
			{
				std::cerr << "config.txt is malformed\n";
				continue;
			}

			config_map.insert(std::make_pair(strings[0], strings[1]));
		}
	};

	std::string get_param(std::string param) const {
		if (config_map.count(param) == 0) {
			std::cerr << "param " << param << " not found\n";
			return {};
		}

		return std::get<1>(*(config_map.find(param)));
	}

};

#endif