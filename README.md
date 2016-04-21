[![Build Status](https://circleci.com/gh/mapbox/shelf-pack-cpp.svg?style=svg)](https://circleci.com/gh/mapbox/shelf-pack-cpp)

## shelf-pack-cpp

C++ port of [shelf-pack](https://github.com/mapbox/shelf-pack)

A 2D rectangular [bin packing](https://en.wikipedia.org/wiki/Bin_packing_problem)
data structure that uses the Shelf Best Height Fit heuristic.


### What is it?

`shelf-pack` is a library for packing little rectangles into a big rectangle.  This sounds simple enough,
but finding an optimal packing is a problem with [NP-Complete](https://en.wikipedia.org/wiki/NP-completeness)
complexity.  One useful application of bin packing is to assemble icons or glyphs into a sprite texture.

There are many ways to approach the bin packing problem, but `shelf-pack` uses the Shelf Best
Height Fit heuristic.  It works by dividing the total space into "shelves", each with a certain height.
The allocator packs rectangles onto whichever shelf minimizes the amount of wasted vertical space.

`shelf-pack` is simple, fast, and works best when the rectangles have similar heights (icons and glyphs
are like this).  It is not a generalized bin packer, and can potentially waste a lot of space if the
rectangles vary significantly in height.


### Usage

#### Basic

```cpp
#include <shelf-pack.hpp>
#include <iostream>

using namespace mapbox;

void main(void) {

    // Initialize the sprite with a width and height..
    ShelfPack sprite(64, 64);

    // Pack bins one at a time..
    for (int i = 0; i < 5; i++) {
        optional<Bin> bin = sprite.packOne(10, 10);   // request width, height
        // `packOne` returns an `Bin` struct with `x`, `y`, `w`, `h` members..

        if (bin) {
            std::cout << "bin packed at " << bin->x << ", " << bin->y << std::endl;
        } else {
            std::cout << "out of space" << std::endl;
        }
    }

    // Clear sprite and start over..
    sprite.clear();

    // Or, resize sprite by passing larger dimensions..
    sprite.resize(128, 128);   // width, height
}
```


#### Batch packing

```cpp
#include <shelf-pack.hpp>
#include <iostream>

void main(void) {

    // If you don't want to think about the size of the sprite,
    // the `autoResize` option will allow it to grow as needed..
    ShelfPack::ShelfPackOptions options;
    options.autoResize = true;
    ShelfPack sprite(10, 10, options);

    // Bins can be allocated in batches..
    // Each bin should be initialized with `w`, `h` (width, height)..
    std::vector<Bin> bins;
    bins.emplace_back(10, 10);
    bins.emplace_back(10, 12);
    bins.emplace_back(10, 12);
    bins.emplace_back(10, 10);

    std::vector<Bin> results = sprite.pack(bins);
    // `pack` returns a vector of packed bin, with `x`, `y`, `w`, `h` values..

    for (const auto& bin : results) {
        std::cout << "bin packed at " << bin.x << ", " << bin.y << std::endl;
    }


    // If you don't mind letting ShelfPack modify your objects,
    // the `inPlace` option will assign `x` and `y` values to the incoming Bins.
    // Fancy!
    std::vector<Bin> moreBins;
    moreBins.emplace_back(12, 24);
    moreBins.emplace_back(12, 12);
    moreBins.emplace_back(10, 10);

    ShelfPack::PackOptions options;
    options.inPlace = true;

    sprite.pack(moreBins, options);
    for (const auto& bin_ : moreBins) {
        std::cout << "bin packed at " << bin_.x << ", " << bin_.y << std::endl;
    }

}

```

### Documentation

Complete API documentation is here:  http://mapbox.github.io/shelf-pack/docs/


### See also

J. Jylänky, "A Thousand Ways to Pack the Bin - A Practical
Approach to Two-Dimensional Rectangle Bin Packing,"
http://clb.demon.fi/files/RectangleBinPack.pdf, 2010
