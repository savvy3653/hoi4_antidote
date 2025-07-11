#include "notrat_.h"

#include <iostream>

int main(int argc, char* argv[]) {
    Rat rat;
    auto res = rat.start();
    if (!res) {
        std::cout << res.error();
    }

    return 0;
}