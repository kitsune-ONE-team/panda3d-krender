#include "camera.h"
#include "pandaNode.h"
#include "shaderInput.h"
#include "texture.h"
#include "windowProperties.h"

#include "krender/core/render_pipeline.h"


TypeHandle RenderPipeline::_type_handle;

void RenderPipeline::add_render_pass(char* name, Shader* shader) {
    RenderPass* pass;
    unsigned int num_passes = _passes.size();

    if (num_passes == 0) {  // initial render pass
        pass = new RenderPass(name, num_passes, _win, _camera, _has_srgb, _has_alpha);

        // setup camera which which captures scene
        // and renders into FBO using default camera lens
        NodePath cam = pass->get_camera();
        ((Camera*) cam.node())->set_scene(get_scene());
        ((Camera*) cam.node())->set_camera_mask(1 << 0);

    } else {  // other render passes
        pass = new RenderPass(name, num_passes, _win, _camera2d, _has_srgb, _has_alpha);

        // setup plane from previous render pass
        NodePath prev_plane = _passes.back()->get_card();
        // prev_plane.detach_node();  // detach previous plane from screen

        // pass textures from previous render pass in the current render pass
        for (unsigned int i = 0; i < _passes.back()->get_num_textures(); i++) {
            Texture* t = _passes.back()->get_texture(i);
            prev_plane.set_shader_input(ShaderInput(t->get_name(), t));
        }
        if (shader != nullptr)
            prev_plane.set_shader(shader, 100);

        // setup projection camera which captures plane from previous render pass
        // and renders into FBO
        NodePath cam = pass->get_camera();
        ((Camera*) cam.node())->set_scene(prev_plane);
    }

    // show current plane on screen
    NodePath plane = pass->get_card();
    plane.reparent_to(_render2d);

    _passes.push_back(pass);
}

void RenderPipeline::reload_shaders() {
    _configure();
    for (int i = 0; i < _passes.size(); i++) {
        _passes.at(i)->reload_shader();
    }
}
