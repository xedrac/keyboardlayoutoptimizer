#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "configuration.h"

using namespace std;


Configuration::Configuration()
{
    load("conf/base_effort.conf");
}

// read/parse configuration file(s) into into memory
bool Configuration::load(const std::string &base_config_file)
{
    _base_effort.clear();

    FILE *fp = fopen(base_config_file.c_str(), "r");
    if (!fp) {
        return false;
    }

    vector<string> tokens;
    char buf[4096];
    while (fgets(buf, sizeof(buf)-1, fp)) {
        string line = util::trim(buf); 
        // skip comment lines
        if (line.empty() || line[0] == '#')
            continue;

        tokens = util::split(line);
        for (size_t i=0; i<tokens.size(); i++) {
            _base_effort.push_back( strtol(tokens[i].c_str(), 0, 10) );
        }
    }

    fclose(fp);

    if (_base_effort.size() != NUMKEYS) {
        printf("Warning, base_effort config file contains %lu values, but layouts are %d values long\n", _base_effort.size(), NUMKEYS);
        return false;
    }

    return true;
}
