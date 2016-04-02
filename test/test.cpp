#include <shelf-pack.hpp>

#include <cassert>
#include <iostream>

using namespace mapbox;

void testPack1() {
    std::cout << "batch pack allocates same height bins on existing shelf";

    ShelfPack sprite(64, 64);
    std::vector<Bin> bins, results;

    bins.emplace_back(10, 10);
    bins.emplace_back(10, 10);
    bins.emplace_back(10, 10);
    results = sprite.pack(bins);

    assert(results[0].x ==  0 && results[0].y == 0 && results[0].w == 10 && results[0].h == 10);
    assert(results[1].x == 10 && results[1].y == 0 && results[1].w == 10 && results[1].h == 10);
    assert(results[2].x == 20 && results[2].y == 0 && results[2].w == 10 && results[2].h == 10);

    std::cout << " - OK" << std::endl;
}

void testPack2() {
    std::cout << "batch pack allocates larger bins on new shelf";

    ShelfPack sprite(64, 64);
    std::vector<Bin> bins, results;

    bins.emplace_back(10, 10);
    bins.emplace_back(10, 15);
    bins.emplace_back(10, 20);
    results = sprite.pack(bins);

    assert(results[0].x == 0 && results[0].y ==  0 && results[0].w == 10 && results[0].h == 10);
    assert(results[1].x == 0 && results[1].y == 10 && results[1].w == 10 && results[1].h == 15);
    assert(results[2].x == 0 && results[2].y == 25 && results[2].w == 10 && results[2].h == 20);

    std::cout << " - OK" << std::endl;
}

void testPack3() {
    std::cout << "batch pack allocates shorter bins on existing shelf, minimizing waste";

    ShelfPack sprite(64, 64);
    std::vector<Bin> bins, results;

    bins.emplace_back(10, 10);
    bins.emplace_back(10, 15);
    bins.emplace_back(10, 20);
    bins.emplace_back(10,  9);
    results = sprite.pack(bins);

    assert(results[0].x ==  0 && results[0].y ==  0 && results[0].w == 10 && results[0].h == 10);
    assert(results[1].x ==  0 && results[1].y == 10 && results[1].w == 10 && results[1].h == 15);
    assert(results[2].x ==  0 && results[2].y == 25 && results[2].w == 10 && results[2].h == 20);
    assert(results[3].x == 10 && results[3].y ==  0 && results[3].w == 10 && results[3].h ==  9);

    std::cout << " - OK" << std::endl;
}

void testPack4() {
    std::cout << "batch pack sets `x`, `y` properties to bins with `inPlace` option";

    ShelfPack sprite(64, 64);
    std::vector<Bin> bins;

    ShelfPack::PackOptions options;
    options.inPlace = true;

    bins.emplace_back(10, 10);
    bins.emplace_back(10, 10);
    bins.emplace_back(10, 10);
    sprite.pack(bins, options);

    assert(bins[0].x ==  0 && bins[0].y == 0 && bins[0].w == 10 && bins[0].h == 10);
    assert(bins[1].x == 10 && bins[1].y == 0 && bins[1].w == 10 && bins[1].h == 10);
    assert(bins[2].x == 20 && bins[2].y == 0 && bins[2].w == 10 && bins[2].h == 10);

    std::cout << " - OK" << std::endl;
}

void testPack5() {
    std::cout << "batch pack skips bins if not enough room";

    ShelfPack sprite(20, 20);
    std::vector<Bin> bins, results;

    ShelfPack::PackOptions options;
    options.inPlace = true;

    bins.emplace_back(10, 10);
    bins.emplace_back(10, 10);
    bins.emplace_back(10, 30);   // should skip
    bins.emplace_back(10, 10);
    results = sprite.pack(bins, options);

    assert(results[0].x ==  0 && results[0].y ==  0 && results[0].w == 10 && results[0].h == 10);
    assert(results[1].x == 10 && results[1].y ==  0 && results[1].w == 10 && results[1].h == 10);
    assert(results[2].x ==  0 && results[2].y == 10 && results[2].w == 10 && results[2].h == 10);

    assert(bins[0].x ==  0 && bins[0].y ==  0 && bins[0].w == 10 && bins[0].h == 10);
    assert(bins[1].x == 10 && bins[1].y ==  0 && bins[1].w == 10 && bins[1].h == 10);
    assert(bins[2].x == -1 && bins[2].y == -1 && bins[2].w == 10 && bins[2].h == 30);
    assert(bins[3].x ==  0 && bins[3].y == 10 && bins[3].w == 10 && bins[3].h == 10);

    std::cout << " - OK" << std::endl;
}

void testPackOne1() {
    std::cout << "packOne allocates same height bins on existing shelf";

    ShelfPack sprite(64, 64);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0  && bin.y == 0 && bin.w == 10 && bin.h == 10);
    bin = sprite.packOne(10, 10);
    assert(bin.x == 10 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    bin = sprite.packOne(10, 10);
    assert(bin.x == 20 && bin.y == 0 && bin.w == 10 && bin.h == 10);

    std::cout << " - OK" << std::endl;
}

void testPackOne2() {
    std::cout << "packOne allocates larger bins on new shelf";

    ShelfPack sprite(64, 64);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0  && bin.w == 10 && bin.h == 10);
    bin = sprite.packOne(10, 15);
    assert(bin.x == 0 && bin.y == 10 && bin.w == 10 && bin.h == 15);
    bin = sprite.packOne(10, 20);
    assert(bin.x == 0 && bin.y == 25 && bin.w == 10 && bin.h == 20);

    std::cout << " - OK" << std::endl;
}

void testPackOne3() {
    std::cout << "packOne allocates shorter bins on existing shelf, minimizing waste";

    ShelfPack sprite(64, 64);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0  && bin.w == 10 && bin.h == 10);
    bin = sprite.packOne(10, 15);
    assert(bin.x == 0 && bin.y == 10 && bin.w == 10 && bin.h == 15);
    bin = sprite.packOne(10, 20);
    assert(bin.x == 0 && bin.y == 25 && bin.w == 10 && bin.h == 20);
    bin = sprite.packOne(10, 9);
    assert(bin.x == 10 && bin.y == 0 && bin.w == 10 && bin.h == 9);

    std::cout << " - OK" << std::endl;
}

void testNotEnoughRoom() {
    std::cout << "not enough room";

    ShelfPack sprite(10, 10);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    bin = sprite.packOne(10, 10);
    assert(bin.x == -1 && bin.y == -1);

    std::cout << " - OK" << std::endl;
}

void testAutoResize1() {
    std::cout << "autoResize grows sprite dimensions by width then height";

    ShelfPack::ShelfPackOptions options;
    options.autoResize = true;

    ShelfPack sprite(10, 10, options);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    assert(sprite.w == 10 && sprite.h == 10);

    bin = sprite.packOne(10, 10);
    assert(bin.x == 10 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    assert(sprite.w == 20 && sprite.h == 10);

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 10 && bin.w == 10 && bin.h == 10);
    assert(sprite.w == 20 && sprite.h == 20);

    bin = sprite.packOne(10, 10);
    assert(bin.x == 10 && bin.y == 10 && bin.w == 10 && bin.h == 10);
    assert(sprite.w == 20 && sprite.h == 20);

    bin = sprite.packOne(10, 10);
    assert(bin.x == 20 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    assert(sprite.w == 40 && sprite.h == 20);

    std::cout << " - OK" << std::endl;
}

void testAutoResize2() {
    std::cout << "autoResize accommodates big bin requests";

    ShelfPack::ShelfPackOptions options;
    options.autoResize = true;

    ShelfPack sprite(10, 10, options);
    Bin bin;

    bin = sprite.packOne(20, 10);
    assert(bin.x == 0 && bin.y == 0 && bin.w == 20 && bin.h == 10);
    assert(sprite.w == 40 && sprite.h == 10);

    bin = sprite.packOne(10, 40);
    assert(bin.x == 0 && bin.y == 10 && bin.w == 10 && bin.h == 40);
    assert(sprite.w == 40 && sprite.h == 80);

    std::cout << " - OK" << std::endl;
}

void testClear() {
    std::cout << "clear succeeds";

    ShelfPack sprite(10, 10);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    bin = sprite.packOne(10, 10);
    assert(bin.x == -1 && bin.y == -1);

    sprite.clear();
    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0 && bin.w == 10 && bin.h == 10);

    std::cout << " - OK" << std::endl;
}

void testResizeLarger() {
    std::cout << "resize larger succeeds";

    ShelfPack sprite(10, 10);
    Bin bin;

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    assert(sprite.resize(20, 10) == true);   // grow width

    bin = sprite.packOne(10, 10);
    assert(bin.x == 10 && bin.y == 0 && bin.w == 10 && bin.h == 10);
    assert(sprite.resize(20, 20) == true);   // grow height

    bin = sprite.packOne(10, 10);
    assert(bin.x == 0 && bin.y == 10 && bin.w == 10 && bin.h == 10);

    std::cout << " - OK" << std::endl;
}

void testResizeSmaller() {
    std::cout << "resize smaller fails";

    ShelfPack sprite(10, 10);
    assert(sprite.resize(9, 10) == false);   // shrink width
    assert(sprite.resize(10, 9) == false);   // shrink height

    std::cout << " - OK" << std::endl;
}

int main() {
    testPack1();
    testPack2();
    testPack3();
    // testPack4();
    // testPack5();

    testPackOne1();
    testPackOne2();
    testPackOne3();

    testNotEnoughRoom();

    testAutoResize1();
    testAutoResize2();

    testClear();
    testResizeLarger();
    testResizeSmaller();

    return 0;
}
