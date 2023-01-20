#pragma include ".krender_config.inc.glsl"
#pragma include "krender/shader/defines.inc.glsl"

// custom inputs
uniform samplerBuffer light_data;
#if (SUPPORTS_SHADOW_FILTER == 1)
    uniform sampler2DShadow shadowmap;
#else
    uniform sampler2D shadowmap;
#endif


vec3 decode_normal(vec3 color) {
    /*
      Convert 0...1 colors values to -1...1 normal values.
      RGB Color -> XYZ Normal
    */
    return color * 2.0 - 1.0;
}

vec3 encode_normal(vec3 normal) {
    /*
      Convert -1...1 normal values to 0...1 color values.
      XYZ Normal -> RGB Color
    */
    return normal / 2.0 + 0.5;
}

mat3 get_tbn() {
    mat3 tbn = mat3(vert_tan, vert_binorm, vert_norm);
    return tbn;
}

float lambert(vec3 n, vec3 l) {
    /*
      Lambert light calculation.
     */
    float NxL = dot(n, l);
    return max(NxL, 0.0);
}

float attenuation(float light_radius, float light_dist) {
    /*
      Light attenuation.
     */
    // 1 - edge of light radius, 0 - center of light
    float range = min(light_dist / light_radius, 1.0);
    return 1.0 - range;
}

float lrgb(float x) {
    /*
      sRGB to Linear RGB, single channel.
     */
    return (
        (x <= 0.04045) ?
        x / 12.92 :
        pow((x + 0.055) / (1.0 + 0.055), 2.4));
}

float srgb(float x) {
    /*
      Linear RGB to sRGB, single channel.
    */
#if (SRGB_COLOR == 1)
    return x;
#else
    return (
        (x <= 0.0031308) ?
        12.92 * x :
        ((1.0 + 0.055) * pow(x, 1.0 / 2.4) - 0.055));
#endif
}

vec3 lrgb3(vec3 srgb_color) {
    /*
      sRGB to Linear RGB.
    */
    return vec3(
        lrgb(srgb_color.r),
        lrgb(srgb_color.g),
        lrgb(srgb_color.b));
}

vec3 srgb3(vec3 lrgb_color) {
    /*
      Linear RGB to sRGB.
    */
    return vec3(
        srgb(lrgb_color.r),
        srgb(lrgb_color.g),
        srgb(lrgb_color.b));
}

int get_ss_slot(vec3 dir) {
    /*
      Get the affected by direction Shadow Source slot of the specific light.
    */
    vec3 dir_abs = abs(dir);
    float axis_max = max(max(dir_abs.x, dir_abs.y), dir_abs.z);
    if (dir_abs.x >= axis_max - 0.00001) return dir.x >= 0.0 ? 0 : 1;
    if (dir_abs.y >= axis_max - 0.00001) return dir.y >= 0.0 ? 2 : 3;
    return dir.z >= 0.0 ? 4 : 5;
}

float process_shadow(int ss0_slot, vec3 light_vec, float light_dist) {
    /*
      https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    */
    // find a single Shadow Source slot to process
    int ss_slot = ss0_slot + get_ss_slot(-light_vec);
    int index = ((LIGHT_INFO_SIZE * MAX_LIGHTS) + (SHADOW_SOURCE_INFO_SIZE * ss_slot)) / RGBA32;
    vec4 ss_mvp0 = texelFetch(light_data, index++);
    vec4 ss_mvp1 = texelFetch(light_data, index++);
    vec4 ss_mvp2 = texelFetch(light_data, index++);
    vec4 ss_mvp3 = texelFetch(light_data, index++);
    vec4 ss_uv = texelFetch(light_data, index++);

    // parse data
    mat4 ss_mvp = mat4(ss_mvp0, ss_mvp1, ss_mvp2, ss_mvp3);

    // light-space fragment position
    vec4 light_clip = ss_mvp * vec4(vert_pos, 1.0);

    // perform perspective divide
    vec3 shadow_uv = light_clip.xyz / light_clip.w;

    // transform to 0...1 range
    shadow_uv = shadow_uv * 0.5 + 0.5;

    // get tile UV on shadowmap atlas
    vec3 tile_uv = vec3(shadow_uv.xy * ss_uv.zw + ss_uv.xy, shadow_uv.z);

    vec2 texel_size = 1.0 / textureSize(shadowmap, 0);

#if (SUPPORTS_SHADOW_FILTER == 1)
    float bias = 0.01;
#else
    float bias = 0.01;
#endif

    float light_shadow = 0.0;
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            vec2 off_uv = vec2(x, y) * texel_size;

            // get closest depth value from light's perspective (using 0...1 range)
#if (SUPPORTS_SHADOW_FILTER == 1)
            float ray_length = texture(shadowmap, tile_uv.xyz + vec3(off_uv, 0));
#else
            float ray_length = texture(shadowmap, tile_uv.xy + off_uv).x;
#endif

            // light ray penetration depth
            // the deeper it goes inside - the darker the shadows become
            float depth = max(tile_uv.z - ray_length, 0.0);

            // add shadow, greater - darker
            // will be inverted for multiplication later
            light_shadow += (depth > bias) ? 1.0 : 0.0;

        }
    }

    return 1 - clamp(light_shadow / 9.0, 0.0, 1.0);
}

vec4 process_light(int light_slot, vec3 normal) {
    // load and parse data
    int index = LIGHT_INFO_SIZE * light_slot / RGBA32;
    vec4 lights1 = texelFetch(light_data, index + 1);
    vec3 light_col = lights1.rgb;
    float light_radius = lights1.a;

    if (light_radius == 0) return vec4(0.0);

    vec4 lights0 = texelFetch(light_data, index + 0);
    int ss0_slot = int(lights0.x);
    vec3 light_pos = lights0.yzw;

    // prepare input values
    vec3 light_vec = normalize(light_pos - vert_pos);
    float light_dist = distance(light_pos, vert_pos);

    // calculate lights and shadows
    float light_power = lambert(light_vec, normal);
    float light_attenuation = attenuation(light_radius, light_dist);
    float light_shadow = 1;
    if (ss0_slot >= 0 && light_power >= 0.001) {
        light_shadow = process_shadow(ss0_slot, light_vec, light_dist);
    }

    // colorize
    float lightness = light_power * light_attenuation * light_shadow;
    return vec4(light_col * lightness, lightness);
}

vec4 process_shading(vec3 normal) {
    vec4 shading = vec4(0.0, 0.0, 0.0, 0.0);
    int lights = 0;
    for (int light_slot = 0; light_slot < MAX_LIGHTS; light_slot++) {
        vec4 light_shading = process_light(light_slot, normal);
        shading += light_shading;

        if (light_shading.a >= 0.001) lights++;
        if (lights >= 2) break;
    }
    return shading;
}
