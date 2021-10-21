#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <filesystem>
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
		return std::filesystem::exists(config_filename);
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

	std::string get_value(std::string key) const {
		if (config_map.count(key) == 0) {
			throw std::string("key " + key + " not found\n");
		}

		return std::get<1>(*(config_map.find(key)));
	}

	bool has_key(std::string key) const {
		bool hasKey = false;
		if (config_map.count(key) == 1) {
			hasKey = true;
		}

		return hasKey;
	}

};

#endif