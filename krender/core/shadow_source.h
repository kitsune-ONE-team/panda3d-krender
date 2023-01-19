#ifndef CORE_SHADOW_SOURCE_H
#define CORE_SHADOW_SOURCE_H

#include "numeric_types.h"

#define SHADOW_SOURCE_INFO_SIZE ((4 * 4 * 4) + (4 * 4))
#define SHADOW_SOURCE_DEF_SIZE (4 + SHADOW_SOURCE_INFO_SIZE)


struct ShadowSourceUnpacked_s {
    // written by LightManager
    PN_float32 slot;
    // written by ShadowSource
    PN_float32 mvp[4 * 4];  // mat4
    PN_float32 uv[4];  // vec4
};

struct ShadowSourcePacked_s {
    PN_float32 slot;
    unsigned char ss_info[SHADOW_SOURCE_INFO_SIZE];
};

struct ShadowSourceInfo_s {
    PN_float32 mvp[4 * 4];
    PN_float32 uv[4];
};

union ShadowSourceDef {
    struct ShadowSourceUnpacked_s unpacked;
    struct ShadowSourcePacked_s packed;
    unsigned char data[SHADOW_SOURCE_DEF_SIZE];
};

union ShadowSourceInfo {
    struct ShadowSourceInfo_s info;
    unsigned char data[SHADOW_SOURCE_INFO_SIZE];
};

#endif
