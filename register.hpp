#include <iostream>

using namespace std;

class Register {
    public:
        Register();
        Register(int size);
        ~Register();

    private:
        vector<double> storage;

};