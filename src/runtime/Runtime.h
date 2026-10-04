#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include "runtime/ExpressionEvaluator.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

namespace embx::runtime {

// Logical EmbX layout quantities use LayoutSize (uint64_t); Reader offsets remain
// host-sized because they index actual in-memory input containers.

enum class Endian { Little, Big, Native };

class Reader {
    const std::vector<uint8_t>& d_;
    size_t p_ = 0;
    size_t limit_ = 0;
    Endian e_ = Endian::Little;
    std::vector<size_t> limits_;
    uint64_t bit_container_ = 0;
    size_t bit_remaining_ = 0;
public:
    explicit Reader(const std::vector<uint8_t>& d);

    size_t pos() const { return p_; }
    size_t size() const { return d_.size(); }
    size_t limit() const { return limit_; }
    size_t remaining() const { return limit_ - p_; }
    bool bounded() const { return limit_ < d_.size(); }

    void endian(Endian e) { e_ = e; }
    Endian endian() const { return e_; }

    void seek(size_t p);
    void seekRoot(size_t p);
    void align(size_t a);

    uint64_t uint(size_t n);
    int64_t sint(size_t n);
    float f32();
    double f64();
    std::vector<uint8_t> bytes(size_t n);

    void beginBits(size_t totalBits);
    uint64_t bits(size_t width);
    void endBits();

    void pushLimit(size_t n);
    void pushLimitEnd(size_t absolute_end);
    void popLimit();

    struct State {
        size_t pos = 0;
        size_t limit = 0;
        Endian endian = Endian::Little;
        std::vector<size_t> limits;
        uint64_t bitContainer = 0;
        size_t bitRemaining = 0;
    };

    State state() const { return State{p_, limit_, e_, limits_, bit_container_, bit_remaining_}; }
    void restore(const State& s) {
        if (s.pos > s.limit || s.limit > d_.size()) throw std::out_of_range("invalid reader state");
        if (s.bitRemaining > 64) throw std::out_of_range("invalid reader bit state");
        size_t previous = s.limit;
        for (auto it = s.limits.rbegin(); it != s.limits.rend(); ++it) {
            if (*it < previous || *it > d_.size())
                throw std::out_of_range("invalid reader limit stack");
            previous = *it;
        }
        if (!s.limits.empty() && s.pos > s.limits.front())
            throw std::out_of_range("reader position exceeds innermost limit");
        if (s.limits.empty() && s.limit != d_.size())
            throw std::out_of_range("invalid root reader limit");
        if (!s.limits.empty() && s.limits.back() != d_.size())
            throw std::out_of_range("invalid reader limit root");
        p_ = s.pos;
        limit_ = s.limit;
        e_ = s.endian;
        limits_ = s.limits;
        bit_container_ = s.bitContainer;
        bit_remaining_ = s.bitRemaining;
    }
    size_t mark() const { return p_; }
    void restore(size_t p) { seek(p); }
};

struct CallbackResult {
    bool invoked = false;
    bool success = false;
    std::string error;
};

// One runtime callback registry is shared by both codec directions. The
// callback argument vector is identical for encode and decode; only the codec
// context (Reader or encoder output state) differs.
using DecodeCallback = std::function<bool(const std::string&, Reader&, const Environment&, const std::vector<Value>&, std::string&)>;
using EncodeCallback = std::function<bool(const std::string&, std::vector<uint8_t>&, size_t&, const Environment&, const std::vector<Value>&, std::string&)>;

class CallbackRegistry {
    std::unordered_map<std::string, DecodeCallback> decode_;
    std::unordered_map<std::string, EncodeCallback> encode_;
public:
    void add(std::string n, DecodeCallback cb);
    void add(std::string n, EncodeCallback cb);
    CallbackResult call(const std::string& n, Reader& r, const Environment& env = {}, const std::vector<Value>& args = {}) const;
    CallbackResult call(const std::string& n, std::vector<uint8_t>& out, size_t& pos, const Environment& env, const std::vector<Value>& args) const;
};

} // namespace embx::runtime
