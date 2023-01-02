#ifndef CORE_SHADOW_SOURCE_H
#define CORE_SHADOW_SOURCE_H

#define SHADOW_SOURCE_INFO_SIZE ((4 * 4 * 4) + (4 * 4))
#define SHADOW_SOURCE_DEF_SIZE (4 + SHADOW_SOURCE_INFO_SIZE)


struct ShadowSourceUnpacked_s {
    // written by LightManager
    int32_t slot;
    // written by ShadowSource
    int32_t mvp[4 * 4];  // mat4
    int32_t uv[4];  // vec4
};

struct ShadowSourcePacked_s {
    int32_t slot;
    unsigned char* data[SHADOW_SOURCE_INFO_SIZE];
};

struct ShadowSourceInfo_s {
    int32_t mvp[4 * 4];
    int32_t uv[4];
};

union ShadowSourceDef {
    struct ShadowSourceUnpacked_s unpacked;
    struct ShadowSourcePacked_s packed;
    unsigned char* data[SHADOW_SOURCE_DEF_SIZE];
};

union ShadowSourceInfo {
    struct ShadowSourceInfo_s info;
    unsigned char* data[SHADOW_SOURCE_INFO_SIZE];
};

#endif
