/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *
 */

#include <iostream>
#include <city.h>

int main(int argc, char *argv[])
{
    std::string str;

    if (argc > 1) {
        str = argv[1];
        std::cout << CityHash64(str.c_str(), str.length()) << "\n";
    } else {
        while (std::getline(std::cin, str)) {
            std::cout << CityHash64(str.c_str(), str.length()) << "\n";
        }
    }

    return 0;
}

