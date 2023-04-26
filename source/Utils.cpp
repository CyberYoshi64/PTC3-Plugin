#include "Utils.hpp"

int dateTimeToString(char *out, u64 in) {
    // Conversion code adapted from https://stackoverflow.com/questions/21593692/convert-unix-timestamp-to-date-without-system-libs
    // (original author @gnif under CC-BY-SA 4.0)
    u64 ms = in;
    u32 sec, min, hr, day, yr, mth;
    sec = ms / 1000; ms %= 1000;
    min = sec / 60; sec %= 60;
    hr = min / 60;  min %= 60;
    day = hr / 24;  hr %= 24;

    yr = 1900; // osGetTime starts in 1900

    while(true) {
        bool leap = (!(yr % 4) && ((yr % 100) || !(yr % 400)));
        u16 daycnt = leap ? 366 : 365;
        if(day >= daycnt) {
            day -= daycnt; yr++;
        } else {
            static const u8 perMth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
            for(mth = 0; mth < 12; ++mth) {
                u8 dim = perMth[mth];
                if (mth == 1 && leap) ++dim;
                if (day >= dim)
                    day -= dim;
                else
                    break;
            }
            break;
        }
    }
    day++; mth++;
    return sprintf(out, "%04lu%02lu%02lu%02lu%02lu%02lu%03lu",yr,mth,day,hr,min,sec,ms);
}
