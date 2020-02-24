#include <mapbox/shelf-pack.hpp>

#include <iostream>
#include <stdlib.h>
#include <stdexcept>
#include <ctime>

using namespace mapbox;


int32_t N = 5000000;
int32_t dim = 1000000;

std::vector<Bin> fixedBoth;
std::vector<Bin> randWidth;
std::vector<Bin> randHeight;
std::vector<Bin> randBoth;

const int32_t sizes[4] = {12, 16, 20, 24};

int32_t randSize() {
    int32_t i = rand() % 4;
    return sizes[i];
}

void generateData() {
    std::cout << "N = " << N;
    int32_t w, h, i;
    for (i = 0; i < N; i++) {
        w = randSize();
        h = randSize();
        fixedBoth.emplace_back(-1, 12, 12);
        randWidth.emplace_back(-1, w, 12);
        randHeight.emplace_back(-1, 12, h);
        randBoth.emplace_back(-1, w, h);
    }
    std::cout << " - OK" << std::endl;
}

void benchPackOne1() {
    std::cout << "ShelfPack packOne() fixed size bins" << std::endl;
    ShelfPack sprite(dim, dim);
    Bin* ok;

    std::clock_t start = std::clock();
    for (int32_t j = 0; j < N; j++) {
        ok = sprite.packOne(-1, fixedBoth[j].w, fixedBoth[j].h);
        if (!ok) throw std::runtime_error("out of space");
    }

    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}


void benchPackOne2() {
    std::cout << "ShelfPack packOne() random size bins" << std::endl;
    ShelfPack sprite(dim, dim);
    Bin* ok;

    std::clock_t start = std::clock();
    for (int32_t j = 0; j < N; j++) {
        ok = sprite.packOne(-1, randWidth[j].w, randWidth[j].h);
        if (!ok) throw std::runtime_error("out of space");
    }

    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}

void benchPackOne3() {
    std::cout << "ShelfPack packOne() random height bins" << std::endl;
    ShelfPack sprite(dim, dim);
    Bin* ok;

    std::clock_t start = std::clock();
    for (int32_t j = 0; j < N; j++) {
        ok = sprite.packOne(-1, randHeight[j].w, randHeight[j].h);
        if (!ok) throw std::runtime_error("out of space");
    }

    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}

void benchPackOne4() {
    std::cout << "ShelfPack packOne() random height and width bins" << std::endl;
    ShelfPack sprite(dim, dim);
    Bin* ok;

    std::clock_t start = std::clock();
    for (int32_t j = 0; j < N; j++) {
        ok = sprite.packOne(-1, randBoth[j].w, randBoth[j].h);
        if (!ok) throw std::runtime_error("out of space");
    }

    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}

void benchPack1() {
    std::cout << "ShelfPack batch pack() fixed size bins" << std::endl;
    ShelfPack sprite(dim, dim);

    std::clock_t start = std::clock();
    sprite.pack(fixedBoth);
    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}

void benchPack2() {
    std::cout << "ShelfPack batch pack() random width bins" << std::endl;
    ShelfPack sprite(dim, dim);

    std::clock_t start = std::clock();
    sprite.pack(randWidth);
    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}

void benchPack3() {
    std::cout << "ShelfPack batch pack() random height bins" << std::endl;
    ShelfPack sprite(dim, dim);

    std::clock_t start = std::clock();
    sprite.pack(randHeight);
    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}

void benchPack4() {
    std::cout << "ShelfPack batch pack() random height and width bins" << std::endl;
    ShelfPack sprite(dim, dim);

    std::clock_t start = std::clock();
    sprite.pack(randBoth);
    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    std::cout << "duration: " << duration << std::endl;
}


int main() {
    std::cout << std::endl << "generateData()" << std::endl << std::string(70, '-') << std::endl;
    srand (time(NULL));
    generateData();

    std::cout << std::endl << "pack()" << std::endl << std::string(70, '-') << std::endl;
    benchPack1();
    benchPack2();
    benchPack3();
    benchPack4();

    std::cout << std::endl << "packOne()" << std::endl << std::string(70, '-') << std::endl;
    benchPackOne1();
    benchPackOne2();
    benchPackOne3();
    benchPackOne4();

    return 0;
}
