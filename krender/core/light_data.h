#ifndef CORE_LIGHT_DATA_H
#define CORE_LIGHT_DATA_H

#ifdef CPPPARSER  // interrogate
union LightInfo;
union ShadowSourceInfo;
#endif

#include "krender/core/light.h"
#include "krender/core/shadow_source.h"

#define MAX_LIGHTS 16
#define LIGHT_DATA_SIZE ((LIGHT_INFO_SIZE * MAX_LIGHTS) + (SHADOW_SOURCE_INFO_SIZE * MAX_LIGHTS * 6))


struct LightData_s {
    LightInfo lights[MAX_LIGHTS];
    ShadowSourceInfo shadow_sources[MAX_LIGHTS * 6];
};

union LightData {
    struct LightData_s contents;
    unsigned char data[LIGHT_DATA_SIZE];
};

#endif
