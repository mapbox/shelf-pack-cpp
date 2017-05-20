#ifndef SHELF_PACK_HPP
#define SHELF_PACK_HPP

#include <experimental/optional>
#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

namespace mapbox {

template <typename T>
using optional = std::experimental::optional<T>;
using std::experimental::nullopt;

const char * const SHELF_PACK_VERSION = "1.0.0";


struct Bin {
public:
    /**
     * Create a new Bin.
     *
     * @private
     * @class  Shelf
     * @param  {int32_t}  [id=-1]   Unique bin identifier, generated if not provided
     * @param  {int32_t}  [w1=-1]   Width of the new Bin
     * @param  {int32_t}  [h1=-1]   Height of the new Bin
     * @param  {int32_t}  [x1=-1]   X location of the Bin
     * @param  {int32_t}  [y1=-1]   Y location of the Bin
     * @example
     * Bin b(-1, 12, 16, 0, 0);
     */
    explicit Bin(int32_t id1 = -1, int32_t w1 = -1, int32_t h1 = -1, int32_t x1 = -1, int32_t y1 = -1) :
        id(id1), w(w1), h(h1), x(x1), y(y1) {
        if (id == -1) {
            id = getId();
        }
    }

    int32_t id;
    int32_t w;
    int32_t h;
    int32_t x;
    int32_t y;

private:
    int32_t getId() {
        static std::atomic<std::int32_t> next { 0 };
        return ++next;
    };
};


class Shelf {
public:
    /**
     * Create a new Shelf.
     *
     * @private
     * @class  Shelf
     * @param  {int32_t}  y   Top coordinate of the new shelf
     * @param  {int32_t}  w   Width of the new shelf
     * @param  {int32_t}  h   Height of the new shelf
     * @example
     * Shelf shelf(64, 512, 24);
     */
    explicit Shelf(int32_t y, int32_t w, int32_t h) :
        x_(0), y_(y), w_(w), h_(h), wfree_(w) { }

    /**
     * Allocate a single bin into the shelf.
     *
     * @private
     * @param    {int32_t}  id    Unique bin identifier, pass -1 to generate a new one
     * @param    {int32_t}  w     Width of the bin to allocate
     * @param    {int32_t}  h     Height of the bin to allocate
     * @returns  {optional<Bin>}  `Bin` struct with `id`, `x`, `y`, `w`, `h` members
     * @example
     * optional<Bin> result = shelf.alloc(-1, 12, 16);
     */
    optional<Bin> alloc(int32_t id, int32_t w, int32_t h) {
        if (w > wfree_ || h > h_) {
            return nullopt;
        }
        int32_t x = x_;
        x_ += w;
        wfree_ -= w;
        return Bin(id, w, h, x, y_);
    }

    /**
     * Resize the shelf.
     *
     * @private
     * @param    {int32_t}  w  Requested new width of the shelf
     * @returns  {bool}     `true` if resize succeeded, `false` if failed
     * @example
     * shelf.resize(512);
     */
    bool resize(int32_t w) {
        wfree_ += (w - w_);
        w_ = w;
        return true;
    }

    int32_t x() const { return x_; }
    int32_t y() const { return y_; }
    int32_t w() const { return w_; }
    int32_t h() const { return h_; }
    int32_t wfree() const { return wfree_; }

private:
    int32_t x_;
    int32_t y_;
    int32_t w_;
    int32_t h_;
    int32_t wfree_;
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
     * @param  {int32_t}  [w=64]  Initial width of the sprite
     * @param  {int32_t}  [h=64]  Initial width of the sprite
     * @param  {ShelfPackOptions}  [options]
     * @param  {bool} [options.autoResize=false]  If `true`, the sprite will automatically grow
     * @example
     * ShelfPack::ShelfPackOptions options;
     * options.autoResize = false;
     * ShelfPack sprite = new ShelfPack(64, 64, options);
     */
    explicit ShelfPack(int32_t w = 0, int32_t h = 0, const ShelfPackOptions &options = ShelfPackOptions{}) {
        width_ = w > 0 ? w : 64;
        height_ = h > 0 ? h : 64;
        autoResize_ = options.autoResize;
    }


    /**
     * Batch pack multiple bins into the sprite.
     *
     * @param   {vector<Bin>}   bins Array of requested bins - each object should have `w`, `h` values
     * @param   {PackOptions}  [options]
     * @param   {bool} [options.inPlace=false] If `true`, the supplied bin objects will be updated inplace with `x` and `y` values
     * @returns {vector<Bin>}   Array of Bins - each bin is a struct with `x`, `y`, `w`, `h` values
     * @example
     * std::vector<Bin> moreBins;
     * moreBins.emplace_back(-1, 12, 24);
     * moreBins.emplace_back(-1, 12, 12);
     * moreBins.emplace_back(-1, 10, 10);
     *
     * ShelfPack::PackOptions options;
     * options.inPlace = true;
     * std::vector<Bin> results = sprite.pack(moreBins, options);
     */
    std::vector<Bin> pack(std::vector<Bin> &bins, const PackOptions &options = PackOptions{}) {
        std::vector<Bin> results;

        for (auto& bin : bins) {
            if (bin.w && bin.h) {
                optional<Bin> allocation = packOne(bin.id, bin.w, bin.h);
                if (!allocation) {
                    continue;
                }
                if (options.inPlace) {
                    bin.x = allocation->x;
                    bin.y = allocation->y;
                }
                results.push_back(*allocation);
            }
        }

        // Shrink the width/height of the sprite to the bare minimum.
        // Since shelf-pack doubles first width, then height when running out of shelf space
        // this can result in fairly large unused space both in width and height if that happens
        // towards the end of bin packing.
        if (shelves_.size()) {
            int32_t w2 = 0;
            int32_t h2 = 0;

            for (auto& shelf : shelves_) {
                h2 += shelf.h();
                w2 = std::max(shelf.w() - shelf.wfree(), w2);
            }

            resize(w2, h2);
        }

        return results;
    }


    /**
     * Pack a single bin into the sprite.
     *
     * @param   {int32_t}  id    Unique bin identifier, pass -1 to generate a new one
     * @param   {int32_t}  w     Width of the bin to allocate
     * @param   {int32_t}  h     Height of the bin to allocate
     * @returns {optional<Bin>}  Bin struct with `id`, `x`, `y`, `w`, `h` members
     * @example
     * optional<Bin> result = sprite.packOne(-1, 12, 16);
     */
    optional<Bin> packOne(int32_t id, int32_t w, int32_t h) {
        int32_t y = 0;
        struct { Shelf* pshelf = NULL; int32_t waste = INT32_MAX; } best;

        // find the best shelf
        for (auto& shelf : shelves_) {
            y += shelf.h();

            // exactly the right height with width to spare, pack it..
            if (h == shelf.h() && w <= shelf.wfree()) {
                count(h);
                return shelf.alloc(id, w, h);
            }
            // not enough height or width, skip it..
            if (h > shelf.h() || w > shelf.wfree()) {
                continue;
            }
            // maybe enough height or width, minimize waste..
            if (h < shelf.h() && w <= shelf.wfree()) {
                int32_t waste = shelf.h() - h;
                if (waste < best.waste) {
                    best.waste = waste;
                    best.pshelf = &shelf;
                }
            }
        }

        if (best.pshelf) {
            count(h);
            return best.pshelf->alloc(id, w, h);
        }

        // add shelf..
        if (h <= (height_ - y) && w <= width_) {
            count(h);
            shelves_.emplace_back(y, width_, h);
            return shelves_.back().alloc(id, w, h);
        }

        // no more space..
        // If `autoResize` option is set, grow the sprite as follows:
        //  * double whichever sprite dimension is smaller (`w1` or `h1`)
        //  * if sprite dimensions are equal, grow width before height
        //  * accomodate very large bin requests (big `w` or `h`)
        if (autoResize_) {
            int32_t h1, h2, w1, w2;

            h1 = h2 = height_;
            w1 = w2 = width_;

            if (w1 <= h1 || w > w1) {   // grow width..
                w2 = std::max(w, w1) * 2;
            }
            if (h1 < w1 || h > h1) {    // grow height..
                h2 = std::max(h, h1) * 2;
            }

            resize(w2, h2);
            return packOne(id, w, h);  // retry
        }

        return nullopt;
    }


    /**
     * Clear the sprite.
     *
     * @example
     * sprite.clear();
     */
    void clear() {
        shelves_.clear();
        stats_.clear();
    }


    /**
     * Resize the sprite.
     *
     * @param   {int32_t}  w  Requested new sprite width
     * @param   {int32_t}  h  Requested new sprite height
     * @returns {bool}     `true` if resize succeeded, `false` if failed
     * @example
     * sprite.resize(256, 256);
     */
    bool resize(int32_t w, int32_t h) {
        width_ = w;
        height_ = h;
        for (auto& shelf : shelves_) {
            shelf.resize(width_);
        }
        return true;
    }

    int32_t width() const { return width_; }
    int32_t height() const { return height_; }

private:
    void count(int32_t h) {
       stats_[h]++;
    }

    int32_t width_;
    int32_t height_;
    bool autoResize_;
    std::vector<Shelf> shelves_;
    std::map<int32_t, int32_t> stats_;
};


}

#endif
