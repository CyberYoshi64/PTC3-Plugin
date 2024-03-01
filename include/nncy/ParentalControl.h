#pragma once
#include <3ds.h>

typedef enum ParentalControlFlags {
    ACS_ENABLED = 0,                // Parental Controls is enabled
    ACS_INTERNET_BROWSER = 1,       // Internet Browser
    ACS_3D_DISPLAY = 2,             // Display of 3D Images
    ACS_SHARING_DATA = 3,           // Sharing Images/Audio/Video/Long Text Data
    ACS_ONLINE_INTERACTION = 4,     // Online Interaction
    ACS_STREETPASS = 5,             // StreetPass
    ACS_FRIEND_REGISTRATION = 6,    // Friend Registration
    ACS_DS_DOWNLOAD_PLAY = 7,       // DS Download Play
    ACS_SHOPPING_SERVICES = 8,      // Shopping Services (eShop/MINT)
    ACS_DISTRIB_VIDEOS = 9,         // Viewing Distributed Videos
    ACS_MIIVERSE_VIEW = 10,         // Miiverse (view)
    ACS_MIIVERSE_POST = 11,         // Miiverse (post)
    ACS_COPPACS = 31                // "Child Online Privacy Protection" - only relevant to USA/Canada
} ParentalControlFlags;

#ifdef __cplusplus
extern "C" {
#endif

Result CFG_GetParentalControlMask(u32* flags);

#ifdef __cplusplus
}
#endif
