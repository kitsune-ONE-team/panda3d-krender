#ifndef CORE_LIGHT_H
#define CORE_LIGHT_H

#define LIGHT_INFO_SIZE (4 + (4 * 3) + (4 * 3) + 4 + 4)
#define LIGHT_DEF_SIZE (4 + 4 + 4 + LIGHT_INFO_SIZE)


struct LightUnpacked_s {
    // written by LightManager
    PN_float32 slot;
    // written by rpLight
    PN_float32 type;
    PN_float32 ies;
    PN_float32 ss0;  // -1 if not casts shadows
    PN_float32 pos[3];  // vec3
    PN_float32 color[3];  // vec3
    // written by rpPointLight
    PN_float32 radius;
    PN_float32 iradius;
};

struct LightPacked_s {
    PN_float32 slot;
    PN_float32 type;
    PN_float32 ies;
    unsigned char data[LIGHT_INFO_SIZE];
};

struct LightInfo_s {
    PN_float32 ss0;
    PN_float32 pos[3];
    PN_float32 color[3];
    PN_float32 radius;
    PN_float32 iradius;
};

union LightDef {
    struct LightUnpacked_s unpacked;
    struct LightPacked_s packed;
    unsigned char data[LIGHT_DEF_SIZE];
};

union LightInfo {
    struct LightInfo_s info;
    unsigned char data[LIGHT_INFO_SIZE];
};

#endif
