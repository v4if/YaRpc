#include <iostream>
#include <string> 
using namespace std;

int main() {
    string str;

    size_t size = 778;
    str.append(reinterpret_cast<char*>(&size), sizeof(size));
    cout << size << " hex : " << hex << size << endl;
    cout << str.size() << endl;
    cout << str << endl;

    size_t de_size;
    std::copy(str.begin(), str.begin() + sizeof(size), reinterpret_cast<char*>(&de_size));

    cout << dec << de_size << " hex : " << hex << de_size << endl;
}
