#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <vector>

class Encryption {
public:
    Encryption();

    ~Encryption();

    void generateKey() const;

    bool encryptFile(const std::string &inputFile) const;

    bool decryptFile(const std::string &inputFile) const;

private:
    unsigned char *key;
};

#endif // ENCRYPTION_H
