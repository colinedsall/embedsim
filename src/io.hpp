#include <iostream>

using namespace std;

struct IOType {
    string name;
    bool enable;
};

class IO
{
    public:
        IO();
        IO(IOType config);
        ~IO();
        void toggleIO();
        string getIOName();

    private:
        IOType config;

};