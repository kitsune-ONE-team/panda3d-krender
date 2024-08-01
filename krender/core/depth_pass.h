#ifndef CORE_DEPTH_PASS_H
#define CORE_DEPTH_PASS_H

#include "krender/core/render_pass.h"


class DepthPass: public RenderPass {
public:
    DepthPass(
        char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
        bool has_srgb=false, bool has_alpha=false, NodePath card=NodePath::not_found());

protected:
    PointerTo<GraphicsOutput> _make_fbo(
        PointerTo<GraphicsWindow> win, bool has_srgb, bool has_alpha,
        unsigned short num_textures, unsigned short sort);
    void _make_textures();
    NodePath _make_camera(PointerTo<GraphicsOutput> fbo, char* name, NodePath camera);
};

#endif
