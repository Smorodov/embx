#pragma once
#include "value/Value.h"
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>

namespace embx::runtime {

inline bool parseAssertionLiteral(const std::string& text, const value::Value& actual, value::Value& expected, std::string& err) {
    if (text.empty()) { err = "empty assertion literal"; return false; }
    try {
        const bool neg = text[0] == '-';
        const std::string body = neg ? text.substr(1) : text;
        if (body.empty()) { err = "invalid assertion literal: " + text; return false; }
        if (std::holds_alternative<std::string>(actual.data)) {
            if (text.size() < 2 || text.front() != '"' || text.back() != '"') { err = "string assertion requires a string literal"; return false; }
            std::string s;
            for (size_t i = 1; i + 1 < text.size(); ++i) {
                char c = text[i];
                if (c != '\\') { s.push_back(c); continue; }
                if (++i + 1 > text.size()) { err = "invalid string assertion escape"; return false; }
                switch (text[i]) {
                    case 'n': s.push_back('\n'); break;
                    case 'r': s.push_back('\r'); break;
                    case 't': s.push_back('\t'); break;
                    case '\\': s.push_back('\\'); break;
                    case '"': s.push_back('"'); break;
                    default: s.push_back(text[i]); break;
                }
            }
            expected = std::move(s); return true;
        }
        if (std::holds_alternative<value::Value::Bytes>(actual.data)) {
            if (text.size() < 2 || text.front() != '"' || text.back() != '"') { err = "bytes assertion requires a string literal"; return false; }
            std::string s;
            for (size_t i = 1; i + 1 < text.size(); ++i) {
                char c = text[i];
                if (c == '\\' && i + 1 < text.size()) { c = text[++i]; }
                s.push_back(c);
            }
            expected = value::Value::Bytes(s.begin(), s.end()); return true;
        }
        if (std::get_if<double>(&actual.data)) {
            char* end = nullptr; errno = 0; const double v = std::strtod(text.c_str(), &end);
            if (errno || !end || *end) { err = "invalid floating assertion literal: " + text; return false; }
            expected = v; return true;
        }
        if (std::get_if<int64_t>(&actual.data)) {
            size_t idx = 0; long long v = std::stoll(text, &idx, 0);
            if (idx != text.size()) { err = "invalid integer assertion literal: " + text; return false; }
            expected = static_cast<int64_t>(v); return true;
        }
        if (std::get_if<uint64_t>(&actual.data)) {
            if (neg) { long long v = std::stoll(text, nullptr, 0); expected = static_cast<int64_t>(v); return true; }
            size_t idx = 0; unsigned long long v = std::stoull(text, &idx, 0);
            if (idx != text.size()) { err = "invalid unsigned assertion literal: " + text; return false; }
            expected = static_cast<uint64_t>(v); return true;
        }
        err = "assertion is not supported for value type"; return false;
    } catch (const std::exception&) {
        err = "invalid assertion literal: " + text; return false;
    }
}

inline bool assertionMatches(const value::Value& actual, const std::string& literal, std::string& err) {
    value::Value expected;
    if (!parseAssertionLiteral(literal, actual, expected, err)) return false;
    return value::equal(actual, expected);
}

} // namespace embx::runtime
