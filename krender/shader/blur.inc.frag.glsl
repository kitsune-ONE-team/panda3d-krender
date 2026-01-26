#pragma include ".krender_config.inc.glsl"
#pragma include "krender/shader/defines.inc.glsl"

#define TWO_PI 6.283185307179586
#define BLUR_SAMPLES 4


float intersect_circle(vec2 d, float radius) {
    float latt = clamp(length(d * vec2(1, 1)) / radius, 0, 1);
    latt = pow(latt, 10.0);
    latt = 1 - latt;
    return latt;

    if (length(d) <= radius) {
        return 1.0;
    }
    return 0.0;
}

vec4 process_blur(sampler2D tex, vec2 uv, float blur_size) {
    float aspect = win_size.x / win_size.y;
    vec2 max_radius = vec2(blur_size, blur_size * aspect);
    float max_length = length(max_radius);

    vec4 blur = texture(tex, uv);
    blur *= 1e-4;
    float blurw = 1.0;
    blurw *= 1e-4;

    for (int ring = 0; ring <= BLUR_SAMPLES; ++ring) {
        int n_samples = max(1, 8 * ring);
        vec2 r = max_radius * ring / float(BLUR_SAMPLES);

        for (int i = 0; i < n_samples; ++i) {
            float phi = i / float(n_samples) * TWO_PI;
            float x_offs = sin(phi);
            float y_offs = cos(phi);

            vec2 blur_uv = uv + vec2(x_offs, y_offs) * r;
            vec4 tex_data = texture(tex, blur_uv);

            float coc_weight = intersect_circle(r, max_length);
            coc_weight *= 1.0 / max(1e-9, 1.0);
            blur += tex_data * coc_weight;
            blurw += coc_weight;
        }
    }

    blur /= max(1e-5, blurw);
    return blur;
}
