#include "config.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <iterator>
#include <cstdlib>

typedef std::vector<std::string> configLine;

Config::Config()
{
    this->debug = false;
    this->localPort = 0;
    this->serverPort = 23232;
    this->serverAddr = "";
    this->tapInterface = "";
}

Config::~Config()
{
    //dtor
}

configLine splitByWhitespace(std::string input) {
    std::istringstream buffer(input);
    configLine ret((std::istream_iterator<std::string>(buffer)), std::istream_iterator<std::string>());
    return ret;
}

std::vector<std::string> split(std::string str,const char* delim)
{
    char* saveptr;
    char *dup = strdup(str.c_str());
    char* token = strtok_r(dup, delim,&saveptr);
    free(dup);

    std::vector<std::string> result;

    while(token != NULL)
    {
        result.push_back(token);
        token = strtok_r(NULL,delim,&saveptr);
    }
    return result;
}

bool Config::loadConfigFile(std::string filename)
{
    /* open the file */
    std::ifstream configFile;
    configFile.open(filename.c_str(), std::ifstream::in);

    /* return on error */
    if(configFile.fail())
        return false;

    /* parse each line */
    std::string line;
    while (std::getline(configFile, line))
    {
        /* split the line by whitespace */
        configLine parts = splitByWhitespace(line);

        /* extract the values */
        if (parts[0] == "debug") {
            this->debug = (parts[1] == "1" ? true : false);
        } else if (parts[0] == "intf") {
            this->tapInterface = parts[1];
        } else if (parts[0] == "server") {
            this->localPort = 0;
            this->serverAddr = parts[1];
            this->serverPort = atoi(parts[2].c_str());
        } else if (parts[0] == "listen") {
            this->localPort = atoi(parts[1].c_str());            
        }
    }

    return true;
}
