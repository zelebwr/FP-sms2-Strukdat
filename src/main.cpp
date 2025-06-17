#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
using namespace std;

int tambahBiasa(const int a, const int b) {
    return a + b;
}

int main(){
    int hasil = tambahBiasa(5, 10);
    cout << "Hasil: " << hasil << endl;
    return 0;
}