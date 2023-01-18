#include "camera.h"
#include "pandaNode.h"
#include "shaderInput.h"
#include "texture.h"
#include "windowProperties.h"

#include "krender/core/render_pipeline.h"


TypeHandle RenderPipeline::_type_handle;

void RenderPipeline::add_render_pass(char* name, Shader* shader) {
    RenderPass* pass;

    if (_passes.size() == 0) {  // initial render pass
        pass = new RenderPass(
            name, (unsigned int) _passes.size(), _win, _camera, _has_srgb, _has_alpha);

        // setup camera which which captures scene
        // and renders into FBO using default camera lens
        NodePath cam = pass->get_camera();
        ((Camera*) cam.node())->set_scene(get_scene());
        ((Camera*) cam.node())->set_camera_mask(1 << 0);

    } else {  // other render passes
        pass = new RenderPass(
            name, (unsigned int) _passes.size(), _win, _camera2d, _has_srgb, _has_alpha);

        // setup plane from previous render pass
        NodePath prev_plane = _passes.back()->get_fbo()->get_texture_card();
        prev_plane.detach_node();  // detach previous plane from screen

        // pass textures from previous render pass in the current render pass
        for (unsigned int i = 0; i < _passes.back()->get_num_textures(); i++) {
            Texture* t = _passes.back()->get_texture(i);
            prev_plane.set_shader_input(ShaderInput(t->get_name(), t));
        }
        prev_plane.set_shader(shader, 100);

        // setup projection camera which captures plane from previous render pass
        // and renders into FBO
        NodePath cam = pass->get_camera();
        ((Camera*) cam.node())->set_scene(prev_plane);
    }

    // show current plane on screen
    NodePath plane = pass->get_fbo()->get_texture_card();
    plane.reparent_to(_render2d);

    _passes.push_back(pass);
}
