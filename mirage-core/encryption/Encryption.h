#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>

class Encryption {
public:
    Encryption();

    ~Encryption();

    void generateKey() const;

    [[nodiscard]] bool encryptFile(const std::string &inputFile) const;

    [[nodiscard]] bool decryptFile(const std::string &inputFile) const;

private:
    unsigned char *key;
};

#endif // ENCRYPTION_H
