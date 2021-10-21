#ifndef SHYNET_CRYPTO_URL_H
#define SHYNET_CRYPTO_URL_H

#include <string>

namespace shynet {
namespace crypto {
    namespace url {
        static char dec2hexChar(short int n)
        {
            if (0 <= n && n <= 9) {
                return char(short('0') + n);
            } else if (10 <= n && n <= 15) {
                return char(short('A') + n - 10);
            } else {
                return char(0);
            }
        }

        static short int hexChar2dec(char c)
        {
            if ('0' <= c && c <= '9') {
                return short(c - '0');
            } else if ('a' <= c && c <= 'f') {
                return short(short(c - 'a') + 10);
            } else if ('A' <= c && c <= 'F') {
                return short(short(c - 'A') + 10);
            } else {
                return -1;
            }
        }

        std::string escapeURL(const std::string& URL)
        {
            std::string result = "";
            for (unsigned int i = 0; i < URL.size(); i++) {
                char c = URL[i];
                if (
                    ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '/' || c == '.') {
                    result += c;
                } else {
                    int j = (short int)c;
                    if (j < 0) {
                        j += 256;
                    }
                    int i1, i0;
                    i1 = j / 16;
                    i0 = j - i1 * 16;
                    result += '%';
                    result += dec2hexChar(short(i1));
                    result += dec2hexChar(short(i0));
                }
            }
            return result;
        }

        std::string deescapeURL(const std::string& URL)
        {
            std::string result = "";
            for (unsigned int i = 0; i < URL.size(); i++) {
                char c = URL[i];
                if (c != '%') {
                    result += c;
                } else {
                    char c1 = URL[++i];
                    char c0 = URL[++i];
                    int num = 0;
                    num += hexChar2dec(c1) * 16 + hexChar2dec(c0);
                    result += char(num);
                }
            }
            return result;
        }
    }
}
}

#endif
