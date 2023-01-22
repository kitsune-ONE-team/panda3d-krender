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
        // get plane from previous render pass
        NodePath prev_plane = _passes.back()->get_result_card();

        pass = new RenderPass(name, num_passes, _win, _camera2d, _has_srgb, _has_alpha, prev_plane);

        // pass textures from the first render pass to the current render pass
        for (unsigned int i = 0; i < _passes.front()->get_num_textures(); i++) {
            Texture* t = _passes.front()->get_texture(i);
            char* s = (char*) malloc((strlen(_passes.front()->get_name()) + strlen(t->get_name().c_str()) + 1) * sizeof(char));
            sprintf(s, "%s_%s", _passes.front()->get_name(), t->get_name().c_str());
            pass->get_source_card().set_shader_input(ShaderInput(std::string(s), t));
        }

        // pass textures from the previous render pass to the current render pass
        for (unsigned int i = 0; i < _passes.back()->get_num_textures(); i++) {
            Texture* t = _passes.back()->get_texture(i);
            char* s = (char*) malloc((strlen("prev") + strlen(t->get_name().c_str()) + 1) * sizeof(char));
            sprintf(s, "prev_%s", t->get_name().c_str());
            pass->get_source_card().set_shader_input(ShaderInput(std::string(s), t));
        }

        if (shader != nullptr)
            pass->get_source_card().set_shader(shader, 100);

        // setup projection camera which captures plane from previous render pass
        // and renders into FBO
        NodePath cam = pass->get_camera();
        ((Camera*) cam.node())->set_scene(prev_plane);
    }

    // show current plane on screen
    NodePath plane = pass->get_result_card();
    plane.reparent_to(_render2d);

    _passes.push_back(pass);
}

NodePath RenderPipeline::get_source_card(char* name) {
    for (unsigned int i = 0; i < _passes.size(); i++) {
        if (strcmp(_passes.at(i)->get_name(), name) == 0) {
            return _passes.at(i)->get_source_card();
        }
    }
    return NodePath::not_found();
}

void RenderPipeline::update() {
    if (_win_w != _win->get_x_size() || _win_h != _win->get_y_size()) {
        _win_w = _win->get_x_size();
        _win_h = _win->get_y_size();
        _configure();
        for (int i = 0; i < _passes.size(); i++) {
            _passes.at(i)->reload_shader();
        }
    }
    LightingPipeline::update();
}
