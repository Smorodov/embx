#include "runtime/Runtime.h"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace embx::runtime {
namespace {
size_t checkedAdd(size_t a, size_t b, const char* what) {
    if (b > std::numeric_limits<size_t>::max() - a) throw std::overflow_error(what);
    return a + b;
}

bool hostLittleEndian() {
    const uint16_t x = 1;
    return *reinterpret_cast<const uint8_t*>(&x) == 1;
}
}

Reader::Reader(const std::vector<uint8_t>& d) : d_(d), limit_(d.size()) {}

void Reader::seek(size_t p) {
    if (p > limit_) throw std::out_of_range("seek beyond reader limit");
    p_ = p;
}

void Reader::seekRoot(size_t p) {
    if (p > d_.size()) throw std::out_of_range("root seek beyond input");
    if (p > limit_) throw std::out_of_range("root seek beyond active reader limit");
    p_ = p;
}

void Reader::align(size_t a) {
    if (a == 0) throw std::invalid_argument("alignment is zero");
    const size_t rem = p_ % a;
    const size_t delta = rem == 0 ? 0 : a - rem;
    seek(checkedAdd(p_, delta, "alignment overflow"));
}

uint64_t Reader::uint(size_t n) {
    if (n == 0 || n > 8) throw std::invalid_argument("integer width must be 1..8 bytes");
    if (n > remaining()) throw std::out_of_range("integer read beyond reader limit");

    const bool little = (e_ == Endian::Little) || (e_ == Endian::Native && hostLittleEndian());
    uint64_t v = 0;
    if (little) {
        for (size_t i = 0; i < n; ++i) v |= uint64_t(d_[p_ + i]) << (8 * i);
    } else {
        for (size_t i = 0; i < n; ++i) v = (v << 8) | d_[p_ + i];
    }
    p_ += n;
    return v;
}

int64_t Reader::sint(size_t n) {
    const uint64_t u = uint(n);
    if (n == 8) return static_cast<int64_t>(u);
    const unsigned bits_n = static_cast<unsigned>(n * 8);
    const uint64_t sign = uint64_t(1) << (bits_n - 1);
    if ((u & sign) == 0) return static_cast<int64_t>(u);
    const uint64_t mask = ~uint64_t(0) << bits_n;
    return static_cast<int64_t>(u | mask);
}

float Reader::f32() {
    const uint32_t u = static_cast<uint32_t>(uint(4));
    float v{};
    std::memcpy(&v, &u, sizeof(v));
    return v;
}

double Reader::f64() {
    const uint64_t u = uint(8);
    double v{};
    std::memcpy(&v, &u, sizeof(v));
    return v;
}

std::vector<uint8_t> Reader::bytes(size_t n) {
    if (n > remaining()) throw std::out_of_range("byte read beyond reader limit");
    std::vector<uint8_t> b(d_.begin() + static_cast<std::ptrdiff_t>(p_),
                           d_.begin() + static_cast<std::ptrdiff_t>(p_ + n));
    p_ += n;
    return b;
}

void Reader::beginBits(size_t totalBits) {
    if (bit_remaining_ != 0) throw std::logic_error("bit container already active");
    if (totalBits == 0 || totalBits > 64) throw std::invalid_argument("bit container width must be 1..64");
    const size_t bytes_needed = (totalBits + 7) / 8;
    if (bytes_needed > remaining()) throw std::out_of_range("bit container read beyond reader limit");

    const bool little = (e_ == Endian::Little) || (e_ == Endian::Native && hostLittleEndian());
    bit_container_ = 0;
    if (little) {
        for (size_t i = 0; i < bytes_needed; ++i)
            bit_container_ |= uint64_t(d_[p_ + i]) << (8 * i);
    } else {
        for (size_t i = 0; i < bytes_needed; ++i)
            bit_container_ = (bit_container_ << 8) | d_[p_ + i];
    }
    p_ += bytes_needed;

    // The meaningful bits occupy the most-significant totalBits positions of
    // the selected byte container. Padding bits at the least-significant end
    // are ignored. This gives one deterministic rule for non-byte-aligned
    // containers and makes endian selection affect the container value.
    const size_t containerBits = bytes_needed * 8;
    if (containerBits > totalBits) bit_container_ >>= (containerBits - totalBits);
    bit_remaining_ = totalBits;
}

uint64_t Reader::bits(size_t width) {
    if (bit_remaining_ == 0) {
        // Standalone bit reads use a container exactly as wide as the request.
        beginBits(width);
    }
    if (width == 0 || width > 64) throw std::invalid_argument("bit width must be 1..64");
    if (width > bit_remaining_) throw std::out_of_range("bit read exceeds active bit container");

    const size_t shift = bit_remaining_ - width;
    uint64_t mask = width == 64 ? ~uint64_t(0) : ((uint64_t(1) << width) - 1);
    const uint64_t v = (bit_container_ >> shift) & mask;
    bit_remaining_ -= width;
    return v;
}

void Reader::endBits() {
    if (bit_remaining_ != 0) throw std::logic_error("bit container not fully consumed");
    bit_container_ = 0;
}

void Reader::pushLimit(size_t n) {
    const size_t end = checkedAdd(p_, n, "block size overflow");
    pushLimitEnd(end);
}

void Reader::pushLimitEnd(size_t absolute_end) {
    if (absolute_end < p_) throw std::out_of_range("block limit precedes reader position");
    if (absolute_end > limit_) throw std::out_of_range("block exceeds reader limit");
    limits_.push_back(limit_);
    limit_ = absolute_end;
}

void Reader::popLimit() {
    if (limits_.empty()) throw std::logic_error("no active reader limit");
    if (p_ > limit_) throw std::logic_error("reader position exceeds active limit");
    limit_ = limits_.back();
    limits_.pop_back();
}

void CallbackRegistry::add(std::string n, DecodeCallback cb) {
    if (n.empty()) throw std::invalid_argument("callback name is empty");
    if (!cb) throw std::invalid_argument("callback is empty");
    decode_[std::move(n)] = std::move(cb);
}

void CallbackRegistry::add(std::string n, EncodeCallback cb) {
    if (n.empty()) throw std::invalid_argument("callback name is empty");
    if (!cb) throw std::invalid_argument("callback is empty");
    encode_[std::move(n)] = std::move(cb);
}

CallbackResult CallbackRegistry::call(const std::string& n, Reader& r, const Environment& env, const std::vector<Value>& args) const {
    const auto it = decode_.find(n);
    if (it == decode_.end()) return {false, false, "unknown decode callback: " + n};
    const Reader::State saved = r.state();
    std::string error;
    try {
        if (!it->second(n, r, env, args, error)) {
            r.restore(saved);
            return {true, false, error.empty() ? "decode callback failed: " + n : error};
        }
        return {true, true, {}};
    } catch (const std::exception& ex) {
        r.restore(saved);
        return {true, false, ex.what()};
    }
}

CallbackResult CallbackRegistry::call(const std::string& n, std::vector<uint8_t>& out, size_t& pos, const Environment& env, const std::vector<Value>& args) const {
    const auto it = encode_.find(n);
    if (it == encode_.end()) return {false, false, "unknown encode callback: " + n};
    const size_t saved = pos;
    const size_t oldSize = out.size();
    std::string error;
    try {
        if (!it->second(n, out, pos, env, args, error)) {
            out.resize(oldSize);
            pos = saved;
            return {true, false, error.empty() ? "encode callback failed: " + n : error};
        }
        if (pos > out.size()) {
            out.resize(oldSize);
            pos = saved;
            return {true, false, "encode callback advanced beyond output: " + n};
        }
        return {true, true, {}};
    } catch (const std::exception& ex) {
        out.resize(oldSize);
        pos = saved;
        return {true, false, ex.what()};
    }
}

} // namespace embx::runtime
