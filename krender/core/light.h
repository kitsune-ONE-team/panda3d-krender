#ifndef CORE_LIGHT_H
#define CORE_LIGHT_H

#define LIGHT_INFO_SIZE (4 + (4 * 3) + (4 * 3) + 4 + 4)
#define LIGHT_DEF_SIZE (4 + 4 + 4 + LIGHT_INFO_SIZE)


struct LightUnpacked_s {
    // written by LightManager
    int32_t slot;
    // written by rpLight
    int32_t type;
    int32_t ies;
    int32_t ss0;  // -1 if not casts shadows
    int32_t pos[3];  // vec3
    int32_t color[3];  // vec3
    // written by rpPointLight
    PN_float32 radius;
    PN_float32 iradius;
};

struct LightPacked_s {
    int32_t slot;
    int32_t type;
    int32_t ies;
    unsigned char* data[LIGHT_INFO_SIZE];
};

struct LightInfo_s {
    int32_t ss0;
    int32_t pos[3];
    int32_t color[3];
    PN_float32 radius;
    PN_float32 iradius;
};

union LightDef {
    struct LightUnpacked_s unpacked;
    struct LightPacked_s packed;
    unsigned char* data[LIGHT_DEF_SIZE];
};

union LightInfo {
    struct LightInfo_s info;
    unsigned char* data[LIGHT_INFO_SIZE];
};

#endif
