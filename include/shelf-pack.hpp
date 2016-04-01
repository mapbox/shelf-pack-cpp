#ifndef SHELF_PACK_HPP
#define SHELF_PACK_HPP

#include <experimental/optional>
#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

namespace mapbox {

class ShelfPack {
public:

    struct ShelfPackOptions {
        bool autoResize;
    };

    struct PackOptions {
        bool inPlace;
    };

    /**
     * Create a new ShelfPack bin allocator.
     *
     * Uses the Shelf Best Height Fit algorithm from
     * http://clb.demon.fi/files/RectangleBinPack.pdf
     *
     * @class  ShelfPack
     * @param  {uint32_t}  [w=64]  Initial width of the sprite
     * @param  {uint32_t}  [h=64]  Initial width of the sprite
     * @param  {ShelfPackOptions}  [options]
     * @param  {bool} [options.autoResize=false]  If `true`, the sprite will automatically grow
     * @example
     * ShelfPack::ShelfPackOptions options;
     * options.autoResize = false;
     * ShelfPack sprite = new ShelfPack(64, 64, options);
     */
    ShelfPack(uint32_t w = 0, uint32_t h = 0, ShelfPackOptions &options = NULL) {
        this.w = w || 64;
        this.h = h || 64;
        this.autoResize = options && options.autoResize;
    };

    ~ShelfPack() { };


    /**
     * Batch pack multiple bins into the sprite.
     *
     * @param   {vector<Bin>}   bins Array of requested bins - each object should have `width`, `height` (or `w`, `h`) properties
     * @param   {PackOptions}  [options]
     * @param   {bool} [options.inPlace=false] If `true`, the supplied bin objects will be updated inplace with `x` and `y` properties
     * @returns {Vector<Bin>}   Array of allocated bins - each bin is an object with `x`, `y`, `w`, `h` properties
     * @example
     * var bins = [
     *     { id: 'a', width: 12, height: 12 },
     *     { id: 'b', width: 12, height: 16 },
     *     { id: 'c', width: 12, height: 24 }
     * ];
     * ShelfPack::PackOptions options;
     * options.inPlace = false;
     * std::vector<Bin> results = sprite.pack(bins, options);
     */
    std::vector<Bin> pack(std::vector<Bin> bins, PackOptions& options = NULL) {
        std::vector<Bin> results;

        for (auto bin : bins) {
            uint32_t w = bin.w || bin.width;
            uint32_t h = bin.h || bin.height;
            if (w && h) {
                std::experimental::optional<Bin> allocation = this.packOne(w, h);
                if (!allocation) {
                    continue;
                }
                if (options && options.inPlace) {
                    bin.x = allocation.x;
                    bin.y = allocation.y;
                }
                results.push_back(allocation);
            }
        }

        return results;
    };


    /**
     * Pack a single bin into the sprite.
     *
     * @param   {uint32_t}  w   Width of the bin to allocate
     * @param   {uint32_t}  h   Height of the bin to allocate
     * @returns {optional<Bin>}  Allocated bin object with `x`, `y`, `w`, `h` properties, or `nullopt` if allocation failed
     * @example
     * optional<Bin> results = sprite.packOne(12, 16);
     */
    std::experimental::optional<Bin> packOne(uint32_t w, uint32_t h) {
        uint32_t y = 0;
        struct { Shelf& shelf = NULL, uint32_t waste = UINT32_MAX } best;

        // find the best shelf
        for (auto& shelf : this.shelves) {
            y += shelf.h;

            // exactly the right height with width to spare, pack it..
            if (h === shelf.h && w <= shelf.free) {
                this.count(h);
                return shelf.alloc(w, h);
            }
            // not enough height or width, skip it..
            if (h > shelf.h || w > shelf.free) {
                continue;
            }
            // maybe enough height or width, minimize waste..
            if (h < shelf.h && w <= shelf.free) {
                uint32_t waste = shelf.h - h;
                if (waste < best.waste) {
                    best.waste = waste;
                    best.shelf = shelf;
                }
            }
        }

        if (best.shelf) {
            this.count(h);
            return best.shelf.alloc(w, h);
        }

        // add shelf..
        if (h <= (this.h - y) && w <= this.w) {
            best.shelf = this.shelves.emplace_back(y, this.w, h);
            this.count(h);
            return best.shelf.alloc(w, h);
        }

        // no more space..
        // If `autoResize` option is set, grow the sprite as follows:
        //  * double whichever sprite dimension is smaller (`w1` or `h1`)
        //  * if sprite dimensions are equal, grow width before height
        //  * accomodate very large bin requests (big `w` or `h`)
        if (this.autoResize) {
            var h1, h2, w1, w2;

            h1 = h2 = this.h;
            w1 = w2 = this.w;

            if (w1 <= h1 || w > w1) {   // grow width..
                w2 = std::max(w, w1) * 2;
            }
            if (h1 < w1 || h > h1) {    // grow height..
                h2 = std::max(h, h1) * 2;
            }

            this.resize(w2, h2);
            return this.packOne(w, h);  // retry
        }

        return std::experimental::optional<Bin>();
    };

    /**
     * Clear the sprite.
     *
     * @example
     * sprite.clear();
     */
    void clear() {
        this.shelves = [];
        this.stats = {};
    };

    /**
     * Resize the sprite.
     * The resize will fail if the requested dimensions are smaller than the current sprite dimensions.
     *
     * @param   {uint32_t}  w  Requested new sprite width
     * @param   {uint32_t}  h  Requested new sprite height
     * @returns {bool} `true` if resize succeeded, `false` if failed
     * @example
     * sprite.resize(256, 256);
     */
    bool resize(uint32_t w, uint32_t h) {
        if (w < this.w || h < this.h) {
            return false;
        }

        this.w = w;
        this.h = h;
        for (auto& shelf : this.shelves) {
            shelf.resize(w);
        }
        return true;
    };


    uint32_t w;
    uint32_t h;

private:
    void count(uint32_t h) {
        this.stats[h]++;
    };

    bool autoResize;
    std::vector<Shelf> shelves;
    std::map<uint32_t, uint32_t> stats;

};



class Shelf {
public:
    /**
     * Create a new Shelf.
     *
     * @private
     * @class  Shelf
     * @param  {uint32_t}  y   Top coordinate of the new shelf
     * @param  {uint32_t}  w   Width of the new shelf
     * @param  {uint32_t}  h   Height of the new shelf
     * @example
     * Shelf shelf(64, 512, 24);
     */
    inline explicit Shelf(uint32_t y, uint32_t w, uint32_t h) :
        x(0), y(y), w(w), free(w), h(h) { };

    /**
     * Allocate a single bin into the shelf.
     *
     * @private
     * @param   {uint32_t}  w   Width of the bin to allocate
     * @param   {uint32_t}  h   Height of the bin to allocate
     * @returns {optional<Bin>}  Allocated bin object with `x`, `y`, `w`, `h` properties, or `nullopt` if allocation failed
     * @example
     * shelf.alloc(12, 16);
     */
    std::experimental::optional<Bin> alloc(uint32_t w, uint32_t h) {
        if (w > this.free || h > this.h) {
            return std::experimental::optional<Bin>();
        }
        var x = this.x;
        this.x += w;
        this.free -= w;
        return std::experimental::optional<Bin>(x, this.y, w, h);
    };

    /**
     * Resize the shelf.
     * The resize will fail if the requested width is smaller than the current shelf width.
     *
     * @private
     * @param   {uint32_t}  w  Requested new width of the shelf
     * @returns {bool} true if resize succeeded, false if failed
     * @example
     * shelf.resize(512);
     */
    bool resize(uint32_t w) {
        if (w < this.w) {
            return false;
        }
        this.free += (w - this.w);
        this.w = w;
        return true;
    };

    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
    uint32_t free;
};


struct Bin {
    inline explicit Bin() { };
    inline explicit Bin(uint32_t x, uint32_t y, uint32_t w, uint32_t h) :
         x(x), y(y), w(w), h(h), width(w), height(h) { };

    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
    uint32_t width;
    uint32_t height;
};


}

#endif
