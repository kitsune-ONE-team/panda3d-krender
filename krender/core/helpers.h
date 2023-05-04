#ifndef CORE_HELPERS_H
#define CORE_HELPERS_H

#include "krender/core/light_data.h"

#define CAMERA_MASK_DEFERRED 1 << 2
#define CAMERA_MASK_SHADOW 1 << 3


void print_light_packet(LightPacket* light);
void print_shadow_source_packet(ShadowSourcePacket* ss);

#endif
