#include <3ds/types.h>

typedef enum : u8 {
    CFG_SOUND_MONO = 0,
    CFG_SOUND_STEREO = 1,
    CFG_SOUND_SURROUND = 2
} cfgSoundMode; // 00070001

typedef struct {
    u16 user_name[11];   // Goofed up null-termination
    u16 isInappropriate;
    u32 ngword_version;
} cfgBlock_000A0000;

typedef struct {
    u8 month;
    u8 day;
} cfgBlock_000A0001;

typedef struct {
    u8 language;
} cfgBlock_000A0002;

typedef struct {
    u8 unknown1;
    u8 unknown2;
    u8 state_id;
    u8 country_id;
} cfgBlock_000B0000;

typedef struct {
    u16 country_name[16][64];
} cfgBlock_000B0001;

typedef struct {
    u16 state_name[16][64];
} cfgBlock_000B0002;

typedef struct {
    s16 latitude;
    s16 longitude;
} cfgBlock_000B0002;

typedef struct {
    u32 parentalMask;           // Parental Controls Restriction Bitmask
    u32 unknown1;
    u8 rating_system;           // Used rating system (ESRB, USK, PEGI, etc.)
    u8 max_allowed_age;         // Maximum age allowed, whereas 20+ = disabled
    u8 secret_question_type;    // 0-5 = Predefined, 6 = custom (see block 0xC0002)
    u8 unknown2;                // Padding?
    char pin[8];                // The PIN is 4 characters, rest is for NULL-term?
    u16 secret_answer[34];      // Answer to secret question (32 + NULL?)
    u8 unknown3[104];           // Padding to fill up to 192 bytes?
} cfgBlock_000C0000;

typedef struct {
    bool registered_email;      // Whether an e-Mail was registered
    char email_address[257];    // Plain e-Mail address (Potentially not NULL-terminated?)
    u16 secret_question[52];    // Custom secret question (51 + NULL?)
    u8 unknown[150];            // Padding to fill up to 512 bytes?
} cfgBlock_000C0002;

typedef struct {
    u16 accepted_eula;
    u16 latest_eula;
} cfgBlock_000D0000;
