#include <iostream>
#include <string>
#include <vector>
using namespace std;

void rc4(vector<uint8_t> &key, vector<uint8_t> &data);
string encrypt(string key, string plaintext);
string decrypt(string key, const char* ciphertext, size_t ciphertextLen);