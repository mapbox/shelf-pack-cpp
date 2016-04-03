#ifndef SHELF_PACK_HPP
#define SHELF_PACK_HPP

#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

namespace mapbox {

struct Bin {
    inline explicit Bin(int32_t w_ = -1, int32_t h_ = -1, int32_t x_ = -1, int32_t y_ = -1) :
        w(w_), h(h_), x(x_), y(y_) { };

    int32_t w;
    int32_t h;
    int32_t x;
    int32_t y;
};


class Shelf {
public:
    /**
     * Create a new Shelf.
     *
     * @private
     * @class  Shelf
     * @param  {int32_t}  y_   Top coordinate of the new shelf
     * @param  {int32_t}  w_   Width of the new shelf
     * @param  {int32_t}  h_   Height of the new shelf
     * @example
     * Shelf shelf(64, 512, 24);
     */
    inline explicit Shelf(int32_t y_, int32_t w_, int32_t h_) :
        x(0), y(y_), w(w_), h(h_), wfree(w_) { };

    /**
     * Allocate a single bin into the shelf.
     *
     * @private
     * @param    {int32_t}  w_   Width of the bin to allocate
     * @param    {int32_t}  h_   Height of the bin to allocate
     * @returns  {Bin}      `Bin` object with `x`, `y`, `w`, `h` properties, all -1 if allocation failed
     * @example
     * shelf.alloc(12, 16);
     */
    Bin alloc(int32_t w_, int32_t h_) {
        if (w_ > wfree || h_ > h) {
            return Bin();
        }
        int32_t x_ = x;
        x += w_;
        wfree -= w_;
        return Bin(w_, h_, x_, y);
    };

    /**
     * Resize the shelf.
     * The resize will fail if the requested width is smaller than the current shelf width.
     *
     * @private
     * @param    {int32_t}  w_  Requested new width of the shelf
     * @returns  {bool}     `true` if resize succeeded, `false` if failed
     * @example
     * shelf.resize(512);
     */
    bool resize(int32_t w_) {
        if (w_ < w) {
            return false;
        }
        wfree += (w_ - w);
        w = w_;
        return true;
    };

    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
    int32_t wfree;
};


class ShelfPack {
public:

    struct ShelfPackOptions {
        inline ShelfPackOptions() : autoResize(false) { };
        bool autoResize;
    };

    struct PackOptions {
        inline PackOptions() : inPlace(false) { };
        bool inPlace;
    };

    /**
     * Create a new ShelfPack bin allocator.
     *
     * Uses the Shelf Best Height Fit algorithm from
     * http://clb.demon.fi/files/RectangleBinPack.pdf
     *
     * @class  ShelfPack
     * @param  {int32_t}  [w_=64]  Initial width of the sprite
     * @param  {int32_t}  [h_=64]  Initial width of the sprite
     * @param  {ShelfPackOptions}  [options]
     * @param  {bool} [options.autoResize=false]  If `true`, the sprite will automatically grow
     * @example
     * ShelfPack::ShelfPackOptions options;
     * options.autoResize = false;
     * ShelfPack sprite = new ShelfPack(64, 64, options);
     */
    ShelfPack(int32_t w_ = 0, int32_t h_ = 0, const ShelfPackOptions &options = ShelfPackOptions{}) {
        w = w_ > 0 ? w_ : 64;
        h = h_ > 0 ? h_ : 64;
        autoResize = options.autoResize;
    };

    ~ShelfPack() { };


    /**
     * Batch pack multiple bins into the sprite.
     *
     * @param   {vector<Bin>}   bins Array of requested bins - each object should have `w`, `h` properties
     * @param   {PackOptions}  [options]
     * @param   {bool} [options.inPlace=false] If `true`, the supplied bin objects will be updated inplace with `x` and `y` properties
     * @returns {vector<Bin>}   Array of Bins - each bin is an object with `x`, `y`, `w`, `h` properties
     * @example
     * var bins = [
     *     { id: 'a', w: 12, h: 12 },
     *     { id: 'b', w: 12, h: 16 },
     *     { id: 'c', w: 12, h: 24 }
     * ];
     * ShelfPack::PackOptions options;
     * options.inPlace = false;
     * std::vector<Bin> results = sprite.pack(bins, options);
     */
    std::vector<Bin> pack(std::vector<Bin> &bins, const PackOptions &options = PackOptions{}) {
        std::vector<Bin> results;

        for (auto& bin : bins) {
            if (bin.w && bin.h) {
                Bin allocation = packOne(bin.w, bin.h);
                if (allocation.x < 0 || allocation.y < 0) {
                    continue;
                }
                if (options.inPlace) {
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
     * @param   {int32_t}  w_   Width of the bin to allocate
     * @param   {int32_t}  h_   Height of the bin to allocate
     * @returns {Bin}      Bin object with `x`, `y`, `w`, `h` properties, all -1 if allocation failed
     * @example
     * Bin results = sprite.packOne(12, 16);
     */
    Bin packOne(int32_t w_, int32_t h_) {
        int32_t y_ = 0;
        struct { Shelf* pshelf = NULL; int32_t waste = INT32_MAX; } best;

        // find the best shelf
        for (auto& shelf : shelves) {
            y_ += shelf.h;

            // exactly the right height with width to spare, pack it..
            if (h_ == shelf.h && w_ <= shelf.wfree) {
                count(h_);
                return shelf.alloc(w_, h_);
            }
            // not enough height or width, skip it..
            if (h_ > shelf.h || w_ > shelf.wfree) {
                continue;
            }
            // maybe enough height or width, minimize waste..
            if (h_ < shelf.h && w_ <= shelf.wfree) {
                int32_t waste = shelf.h - h_;
                if (waste < best.waste) {
                    best.waste = waste;
                    best.pshelf = &shelf;
                }
            }
        }

        if (best.pshelf) {
            count(h_);
            return best.pshelf->alloc(w_, h_);
        }

        // add shelf..
        if (h_ <= (h - y_) && w_ <= w) {
            count(h_);
            shelves.emplace_back(y_, w, h_);
            return shelves.back().alloc(w_, h_);
        }

        // no more space..
        // If `autoResize` option is set, grow the sprite as follows:
        //  * double whichever sprite dimension is smaller (`w1` or `h1`)
        //  * if sprite dimensions are equal, grow width before height
        //  * accomodate very large bin requests (big `w` or `h`)
        if (autoResize) {
            int32_t h1, h2, w1, w2;

            h1 = h2 = h;
            w1 = w2 = w;

            if (w1 <= h1 || w_ > w1) {   // grow width..
                w2 = std::max(w_, w1) * 2;
            }
            if (h1 < w1 || h_ > h1) {    // grow height..
                h2 = std::max(h_, h1) * 2;
            }

            resize(w2, h2);
            return packOne(w_, h_);  // retry
        }

        return Bin();
    };

    /**
     * Clear the sprite.
     *
     * @example
     * sprite.clear();
     */
    void clear() {
        shelves.clear();
        stats.clear();
    };

    /**
     * Resize the sprite.
     * The resize will fail if the requested dimensions are smaller than the current sprite dimensions.
     *
     * @param   {int32_t}  w_  Requested new sprite width
     * @param   {int32_t}  h_  Requested new sprite height
     * @returns {bool}     `true` if resize succeeded, `false` if failed
     * @example
     * sprite.resize(256, 256);
     */
    bool resize(int32_t w_, int32_t h_) {
        if (w_ < w || h_ < h) {
            return false;
        }

        w = w_;
        h = h_;
        for (auto& shelf : shelves) {
            shelf.resize(w);
        }
        return true;
    };


    int32_t w;
    int32_t h;

private:
    void count(int32_t h_) {
       stats[h_]++;
    };

    bool autoResize;
    std::vector<Shelf> shelves;
    std::map<int32_t, int32_t> stats;
};


}

#endif
