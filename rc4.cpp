// #include <iostream>
// #include <string>
// #include <vector>

// using namespace std;

// void rc4(vector<uint8_t> &key, vector<uint8_t> &message) {
//     // Key scheduling algorithm (KSA)
//     vector<uint8_t> S(256);
//     for (int i = 0; i < 256; i++) {
//         S[i] = i;
//     }

//     int j = 0;
//     for (int i = 0; i < 256; i++) {
//         j = (j + S[i] + key[i % key.size()]) % 256;
//         swap(S[i], S[j]);
//     }

//     // Pseudo-random generation algorithm (PRGA)
//     int i = 0;
//     j = 0;
//     for (int k = 0; k < message.size(); k++) {
//         i = (i + 1) % 256;
//         j = (j + S[i]) % 256;
//         swap(S[i], S[j]);
//         int t = (S[i] + S[j]) % 256;
//         message[k] ^= S[t];
//     }
// }

// string encrypt(string key, string plaintext) {
//     vector<uint8_t> keyVec(key.begin(), key.end());
//     vector<uint8_t> plaintextVec(plaintext.begin(), plaintext.end());

//     rc4(keyVec, plaintextVec);

//     return string(plaintextVec.begin(), plaintextVec.end());
// }

// string decrypt(string key, string ciphertext) {
//     vector<uint8_t> keyVec(key.begin(), key.end());
//     vector<uint8_t> ciphertextVec(ciphertext.begin(), ciphertext.end());

//     rc4(keyVec, ciphertextVec);

//     return string(ciphertextVec.begin(), ciphertextVec.end());
// }

// // int main() {
// //     string key = "secret key";
// //     string plaintext = "Hello, world!";

// //     string ciphertext = encrypt(key, plaintext);

// //     // Convert the ciphertext string to a vector of uint8_t
// //     vector<uint8_t> ciphertextVec(ciphertext.begin(), ciphertext.end());

// //     // Print the ciphertext in hexadecimal format
// //     cout << "Ciphertext (hex): ";
// //     for (int i = 0; i < ciphertextVec.size(); i++) {
// //         printf("%02x", ciphertextVec[i]);
// //     }
// //     cout << endl;

// //     string decrypted = decrypt(key, ciphertext);
// //     cout << "Decrypted: " << decrypted << endl;

// //     return 0;
// // }

#include <iostream>
#include <vector>
#include <string>

using namespace std;

void swap(uint8_t& a, uint8_t& b) {
    uint8_t temp = a;
    a = b;
    b = temp;
}

void init(vector<uint8_t>& S, vector<uint8_t>& key) {
    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + S[i] + key[i % key.size()]) % 256;
        swap(S[i], S[j]);
    }
}

void rc4(vector<uint8_t>& key, vector<uint8_t>& data) {
    vector<uint8_t> S(256);
    for (int i = 0; i < 256; i++) {
        S[i] = i;
    }
    init(S, key);

    int i = 0;
    int j = 0;
    for (int k = 0; k < data.size(); k++) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        swap(S[i], S[j]);
        data[k] ^= S[(S[i] + S[j]) % 256];
    }
}

string encrypt(string key, string plaintext) {
    vector<unsigned char> keyVec(key.begin(), key.end());
    vector<unsigned char> plaintextVec(plaintext.begin(), plaintext.end());
    vector<unsigned char> encryptedVec(plaintext.begin(), plaintext.end());

    rc4(keyVec, plaintextVec);

    for (int i = 0; i < encryptedVec.size(); i++) {
        encryptedVec[i] = plaintextVec[i];
    }

    string ciphertext(reinterpret_cast<char*>(encryptedVec.data()), encryptedVec.size());
    return ciphertext;
}

string decrypt(string key, const char* ciphertext, size_t ciphertextLen) {
    vector<unsigned char> keyVec(key.begin(), key.end());
    vector<unsigned char> ciphertextVec(ciphertext, ciphertext + ciphertextLen);
    vector<unsigned char> decryptedVec(ciphertext, ciphertext + ciphertextLen);

    for (int i = 0; i < decryptedVec.size(); i++) {
        decryptedVec[i] = ciphertextVec[i];
    }

    rc4(keyVec, decryptedVec);

    string plaintext(reinterpret_cast<char*>(decryptedVec.data()), decryptedVec.size());
    return plaintext;
}

// int main() {
//     string key = "secret key";
//     string plaintext = "hello world hi";

//     string ciphertext = encrypt(key, plaintext);
//     cout << "Ciphertext: " << hex;
//     for (unsigned char c : ciphertext) {
//         cout << static_cast<int>(c) << " ";
//     }
//     cout << dec << endl;

//     string decrypted = decrypt(key, ciphertext);
//     cout << "Decrypted: " << decrypted << endl;

//     return 0;
// }
