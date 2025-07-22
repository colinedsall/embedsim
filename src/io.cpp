#include "io.hpp"

IO::IO()
{
    cout << "Warning: Default constructor called. IO module will have no features.";
}

IO::IO(IOType config) : config(config) {}

IO::~IO() {}

void IO::toggleIO()
{
    config.enable = (config.enable ? false : true);
}

string IO::getIOName()
{
    return config.name;
}