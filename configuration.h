#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>


#define NUMKEYS 47


class Configuration
{
public:
    Configuration();

    bool load(const std::string &base_effort_file);
    double baseEffort(int keyindex) { return _base_effort[keyindex]; }

private:
    std::vector<double> _base_effort;  // indexed by key index
};


#endif
