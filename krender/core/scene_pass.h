#ifndef CORE_SCENE_PASS_H
#define CORE_SCENE_PASS_H

#include "krender/core/render_pass.h"


class ScenePass: public RenderPass {
public:
    ScenePass(
        char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
        bool has_srgb=false, bool has_alpha=false,
        float sx=1, float sy=1,
        NodePath card=NodePath::not_found());

protected:
    NodePath _make_camera(PointerTo<GraphicsOutput> fbo, char* name, NodePath camera);
};

#endif
