#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>
#include <string.h>
#include <vector>
#include "Utils.hpp"

namespace CTRPluginFramework {
    class Base64 {
    public:
        enum Mode {
            SET_STD = 0,
            SET_URL,
            SET_PTC,
            SET_MAX = 7,
            NO_PADDING = BIT(3),
            STRIP_LINES = BIT(4),
            STRIP_SPACE = BIT(5),
            LAX_DECODE = BIT(6),
        };
        enum State {
            SUCCESS,       // Success
            BAD_SET,       // Invalid character set
            BAD_CHAR,      // Invalid character in Base64 string
            BAD_STRING,    // Badly formatted string
        };
        static const char* Sets[];
        static int EncodeInto(std::string& out, const u8* data, u32 len, u32 mode = Mode::SET_STD);
        static int DecodeInto(std::string& out, const std::string& data, u32 mode = Mode::SET_STD);
        static int EncodeInto(char* out, const u8* data, u32 len, u32 mode = Mode::SET_STD);
        static int DecodeInto(char* out, const std::string& data, u32 mode = Mode::SET_STD);
        static std::string Encode(const u8* data, u32 len, u32 mode = Mode::SET_STD);
        static std::string Decode(const std::string& data, u32 mode = Mode::SET_STD);
        static u32 CalcSizeRaw(u32 len);
        static u32 CalcSize(u32 len);
    };
} // namespace CTRPluginFramework

#endif