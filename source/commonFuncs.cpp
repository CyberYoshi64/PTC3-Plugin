#include "commonFuncs.hpp"

namespace CTRPluginFramework {

    void strcatu16(u16* dest, char* s1, char* s2) {
        while (*s1) *dest++ = *s1++;
        while (*s2) *dest++ = *s2++;
        *dest = '\0';
    }

    int strcmpu8u16(char* ptr1, u16* ptr2) {
        int i = 0;
        u16 char1, char2;
        do {
            char1 = (u16)(ptr1[i]);
            char2 = ptr2[i++];
        } while (char2 == char1 && ptr1[i] && ptr2[i] != u':');
        return char1 - char2;
    }
    
    int strcpyu16u8(u16* ptr1, char* ptr2, int n) {
        int i = 0;
        u8 c;
        while (*ptr1++ && i++<n) {
            c = (u8)(*ptr1);
            *ptr2++ = c;
        }
        return i;
    }

    int strcmpdot(char* ptr1, char* ptr2) {
        int i = 0;
        char char1, char2;
        do {
            char1 = ptr1[i];
            char2 = ptr2[i++];
        } while (char2 == char1 && ptr1[i] && ptr2[i] != ':');
        return char1 - char2;
    }

    void strcpydot(char* dst, char* src, int n) {
        while (*src != ':' && n) {
            *dst++ = *src++;
            n--;
        }
        *dst = '\0';
    }

    u32 strlen_utf8(const std::string& str) {
	    u32 length = 0;
        for (char c:str) if((c&0xC0)!=0x80) ++length;
        return length;
    }
    void strlower(std::string& str) {
        for (u32 i=0; i<str.length(); i++){
            char c = str[i];
            if(c > 64 && c <= 90) str[i] = c+0x20;
        }
    }
    void strupper(std::string& str) {
        for (u32 i=0; i<str.length(); i++){
            char c = str[i];
            if(c > 96 && c <= 122) str[i] = c-0x20;
        }
    }
    u32 osGetUnixTime(){
        // osGetTime returns in millisec
        // 2208988800 -> 70 years
        return ((osGetTime()/1000)-2208988800);
    }
}