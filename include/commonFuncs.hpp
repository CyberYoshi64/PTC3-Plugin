#ifndef COMMONFUNCS_HPP
#define COMMONFUNCS_HPP

#include <CTRPluginFramework.hpp>

namespace CTRPluginFramework {
    void strcatu16(u16* dest, char* s1, char* s2);
    int strcmpu8u16(char* ptr1, u16* ptr2);
    int strcpyu16u8(u16* ptr1, char* ptr2, int n);
    int strcmpdot(char* ptr1, char* ptr2);
    void strcpydot(char* dst, char* src, int n);
    u32 strlen_utf8(const std::string& str);
    void strlower(std::string& str);
    void strupper(std::string& str);
    void strupper16(string16& str);
    u32 osGetUnixTime();
    void strncpyu8u16(u8* str, u16* out, u32 len);
    void strncpyu16u8(u16* str, u8* out, u32 len);
}
#endif