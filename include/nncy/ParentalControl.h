#pragma once
#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ParentalControlFlags {
    ACS_ENABLED = 0,                // Parental Controls is enabled
    ACS_SHARING_DATA = 3,           // Sharing Images/Audio/Video/Long Text Data
    ACS_INTERNET_BROWSER = 4,       // Internet Browser
    ACS_STREETPASS = 5,             // StreetPass
    ACS_FRIEND_REGISTRATION = 6,    // Friend Registration
    ACS_ESHOP_PURCHASES = 8,        // eShop
    ACS_MIIVERSE_VIEW = 10,         // Miiverse (view)
    ACS_MIIVERSE_POST = 11,         // Miiverse (post)
    ACS_COPPACS = 31                // "Child Online Privacy Protection"
} ParentalControlFlags;

Result CFG_GetParentalControlMask(u32* flags);

#ifdef __cplusplus
}
#endif
