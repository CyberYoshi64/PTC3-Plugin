#include "base64.hpp"

namespace CTRPluginFramework {
    const char* Base64::Sets[3] = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=", // Standard
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=", // URL-safe standard
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-*", // X-PETC token (HTTP header field)
    };
    #define B64SETS_COUNT (sizeof(Base64::Sets)/sizeof(char*))

    int Base64::EncodeInto(std::string& out, const u8* data, u32 len, u32 mode){
        if (B64SETS_COUNT <= (mode & Mode::SET_MAX)) return State::BAD_SET;
        const char* set = Sets[mode & Mode::SET_MAX];

        u32 buf = 0, i = 0;

        while (len > 3) {
            buf = data[i]<<16 | data[i+1]<<8 | data[i+2];
            out += set[(buf>>18)&63];
            out += set[(buf>>12)&63];
            out += set[(buf>> 6)&63];
            out += set[(buf>> 0)&63];
            len -= 3; i += 3;
        }
        if (!len) return State::SUCCESS;

        buf = data[i]<<16;
        buf |= len>1 ? (data[i+1]<<8) : 0;
        buf |= len>2 ? (data[i+2]<<0) : 0;
        out += set[(buf>>18)&63];
        out += set[(buf>>12)&63];
        if (len>1 || (set[64] && !(mode & Mode::NO_PADDING)))
            out += len>1 ? set[(buf>>6)&63] : set[64];
        if (len>2 || (set[64] && !(mode & Mode::NO_PADDING)))
            out += len>2 ? set[(buf>>0)&63] : set[64];
        return State::SUCCESS;
    }
    int Base64::EncodeInto(char* out, const u8* data, u32 len, u32 mode){
        if (B64SETS_COUNT <= (mode & Mode::SET_MAX)) return State::BAD_SET;
        const char* set = Sets[mode & Mode::SET_MAX];

        u32 buf = 0, i = 0;

        while (len > 3) {
            buf = data[i]<<16 | data[i+1]<<8 | data[i+2];
            *(out++) = set[(buf>>18)&63];
            *(out++) = set[(buf>>12)&63];
            *(out++) = set[(buf>> 6)&63];
            *(out++) = set[(buf>> 0)&63];
            len -= 3; i += 3;
        }
        if (!len) return State::SUCCESS;

        buf = data[i]<<16;
        buf |= len>1 ? (data[i+1]<<8) : 0;
        buf |= len>2 ? (data[i+2]<<0) : 0;
        *(out++) = set[(buf>>18)&63];
        *(out++) = set[(buf>>12)&63];
        if (len>1 || (set[64] && !(mode & Mode::NO_PADDING)))
            *(out++) = len>1 ? set[(buf>>6)&63] : set[64];
        if (len>2 || (set[64] && !(mode & Mode::NO_PADDING)))
            *(out++) = len>2 ? set[(buf>>0)&63] : set[64];
        return State::SUCCESS;
    }

    int b64__decode(std::string& out, const char* set, char* chunk, u32 mode){
        s32 dec[4] = {64};
        for (u32 j = 0; j < 4; j++) {
            dec[j] = (s32)strchr(set, chunk[j])-(s32)set;
            if (dec[j] < 0) {
                if (mode & Base64::LAX_DECODE)
                    dec[j] = 0;
                else
                    return Base64::BAD_CHAR;
            }
        }
        if (dec[0] == 64 || dec[1] == 64) return Base64::BAD_STRING;
        
        out += (char)(dec[0]<<2 | dec[1]>>4);
        if (dec[2] == 64)
            return -1;
        else
            out += (char)(dec[1]<<4 | dec[2]>>2);
        
        if (dec[3] == 64)
            return -1;
        else
            out += (char)(dec[2]<<6 | dec[3]);

        memset(chunk, set[64], 4);
        return 0;
    }
    int Base64::DecodeInto(std::string& out, const std::string& data, u32 mode){
        if (B64SETS_COUNT <= (mode & Mode::SET_MAX)) return State::BAD_SET;
        const char* set = Sets[mode & Mode::SET_MAX];

        u32 len = data.size(); int res;
        char chunk[4] = {set[64]}; u32 chklen = 0;

        for (u32 i = 0; i < len; i++){
            if ((mode & Mode::STRIP_LINES) && memchr("\n\r\0", data[i], 3)) continue;
            if ((mode & Mode::STRIP_SPACE) && data[i]==' ') continue;
            chunk[chklen++] = data[i];
            if (chklen==4) {
                res = b64__decode(out, set, chunk, mode);
                if (res > 0) return res;
                chklen = 0;
            }
        }
        if (chklen) {
            res = b64__decode(out, set, chunk, mode);
            if (res > 0) return res;
        }
        return State::SUCCESS;
    }

    int b64__decode(char* out, const char* set, char* chunk, u32 mode){
        s32 dec[4] = {64};
        for (u32 j = 0; j < 4; j++) {
            dec[j] = (s32)strchr(set, chunk[j])-(s32)set;
            if (dec[j] < 0) {
                if (mode & Base64::LAX_DECODE)
                    dec[j] = 0;
                else
                    return Base64::BAD_CHAR;
            }
        }
        if (dec[0] == 64 || dec[1] == 64) return Base64::BAD_STRING;
        
        *(out++) = (dec[0]<<2 | dec[1]>>4);
        if (dec[2] == 64)
            return -1;
        else
            *(out++) = (dec[1]<<4 | dec[2]>>2);
        
        if (dec[3] == 64)
            return -1;
        else
            *(out++) = (dec[2]<<6 | dec[3]);

        memset(chunk, set[64], 4);
        return 0;
    }
    int Base64::DecodeInto(char* out, const std::string& data, u32 mode){
        if (B64SETS_COUNT <= (mode & Mode::SET_MAX)) return State::BAD_SET;
        const char* set = Sets[mode & Mode::SET_MAX];

        u32 len = data.size(); int res;
        char chunk[4] = {set[64]}; u32 chklen = 0;

        for (u32 i = 0; i < len; i++){
            if ((mode & Mode::STRIP_LINES) && memchr("\n\r\0", data[i], 3)) continue;
            if ((mode & Mode::STRIP_SPACE) && data[i]==' ') continue;
            chunk[chklen++] = data[i];
            if (chklen==4) {
                res = b64__decode(out, set, chunk, mode);
                out += 3;
                if (res > 0) return res;
                chklen = 0;
            }
        }
        if (chklen) {
            res = b64__decode(out, set, chunk, mode);
            if (res > 0) return res;
        }
        return State::SUCCESS;
    }

    std::string Base64::Encode(const u8* data, u32 len, u32 mode) {
        std::string out;
        EncodeInto(out, data, len, mode);
        return out;
    }
    std::string Base64::Decode(const std::string& data, u32 mode) {
        std::string out;
        DecodeInto(out, data, mode);
        return out;
    }
    u32 Base64::CalcSizeRaw(u32 len){
        return (u32)((len+2)/3)*4;
    }
    u32 Base64::CalcSize(u32 len){
        return (len*3)/4;
    }
}