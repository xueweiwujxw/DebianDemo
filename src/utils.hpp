/**
 * @file utils.hpp
 * @author wlanxww (xueweiwujxw@outlook.com)
 * @brief
 * @date 2024-11-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

constexpr unsigned long long operator"" _KiB(unsigned long long size) {
    return size * 1024;
}

constexpr unsigned long long operator"" _MiB(unsigned long long size) {
    return size * 1024_KiB;
}

constexpr unsigned long long operator"" _GiB(unsigned long long size) {
    return size * 1024_MiB;
}

constexpr unsigned long long operator"" _TiB(unsigned long long size) {
    return size * 1024_GiB;
}

constexpr unsigned long long operator"" _Hz(unsigned long long size) {
    return size;
}

constexpr unsigned long long operator"" _KHz(unsigned long long size) {
    return size * 1000_Hz;
}

constexpr unsigned long long operator"" _MHz(unsigned long long size) {
    return size * 1024_KHz;
}

constexpr unsigned long long operator"" _GHz(unsigned long long size) {
    return size * 1024_MHz;
}

namespace utilsnp
{
    template <std::size_t Length>
    inline uint64_t parse_hex_string(std::string str) {
        if (str.size() > Length)
            throw std::invalid_argument("Invalid input string length: " + str);
        if (Length > 16)
            throw std::overflow_error("Input string length exceeds 64-bit limit: " + str);

        uint64_t result = 0;

        std::istringstream converter(str);
        converter >> std::hex >> result;

        if (converter.fail() || !converter.eof())
            throw std::invalid_argument("Invalid hex digit in input string: " + str);

        return result;
    }

    template <std::size_t Length>
    inline uint64_t parse_binary_string(const std::string &str) {
        if (str.size() > Length)
            throw std::invalid_argument("Invalid input string length: " + str);

        if (Length > 64)
            throw std::overflow_error("Input string length exceeds 64-bit limit: " + str);

        uint64_t result = 0;

        for (char c : str) {
            if (c != '0' && c != '1')
                throw std::invalid_argument("Invalid binary digit in input string: " + str);

            result = (result << 1) | (c - '0');
        }

        return result;
    }

} // namespace utilsnp
