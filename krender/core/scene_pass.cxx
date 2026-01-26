#include "krender/core/scene_pass.h"


ScenePass::ScenePass(
        char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
        bool has_srgb, bool has_alpha, float sx, float sy, NodePath card):
        RenderPass(name, index, win, cam, has_srgb, has_alpha, sx, sy, card) {
    _fbo = _make_fbo(win, has_srgb, has_alpha, true, true, 3, _index);
    _make_textures(true, true, 3);
    char* cam_name = (char*) malloc((strlen(name) + strlen("_camera") + 1) * sizeof(char));
    sprintf(cam_name, "%s_camera", name);
    _cam = _make_camera(_fbo, cam_name, cam);
}

/*
 * Makes a new 3D camera, ShowBase's makeCamera reimplementation.
 */
NodePath ScenePass::_make_camera(PointerTo<GraphicsOutput> fbo, char* name, NodePath camera) {
    NodePath cam = camera.attach_new_node(new Camera(name));

    ((Camera*) cam.node())->set_lens(((Camera*) camera.node())->get_lens());

    DisplayRegion* dr = fbo->make_display_region();
    dr->set_sort(_index);
    dr->set_camera(cam);

    return cam;
}
