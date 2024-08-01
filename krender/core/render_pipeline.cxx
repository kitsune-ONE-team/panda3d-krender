#include "camera.h"
#include "pandaNode.h"
#include "shaderInput.h"
#include "texture.h"
#include "windowProperties.h"

#include "krender/core/depth_pass.h"
#include "krender/core/helpers.h"
#include "krender/core/post_pass.h"
#include "krender/core/render_pipeline.h"
#include "krender/core/scene_pass.h"


TypeHandle RenderPipeline::_type_handle;

RenderPipeline::RenderPipeline(
        GraphicsWindow* window, NodePath render2d, NodePath camera, NodePath camera2d,
        unsigned int index, unsigned int shadow_size,
        bool has_srgb, bool has_pcf, bool has_alpha):
        LightingPipeline(window, camera, has_srgb, has_pcf, shadow_size) {
    _camera2d = camera2d;
    _render2d = render2d;
    _has_alpha = has_alpha;
    _index = index;
    _win_w = window->get_x_size();
    _win_h = window->get_y_size();
}

void RenderPipeline::add_render_pass(
        char* name, unsigned short type, Shader* shader, BitMask32 mask) {
    RenderPass* render_pass;

    if (type == SCENE_PASS) {
        ScenePass* scene_pass = new ScenePass(
            name, _scene_passes.size() + _index, _win, _camera, _has_srgb, _has_alpha);

        // setup camera which which captures scene
        // and renders into FBO using default camera lens
        NodePath cam = scene_pass->get_camera();
        ((Camera*) cam.node())->set_scene(get_scene());
        ((Camera*) cam.node())->set_camera_mask(mask);

        _scene_passes.push_back((RenderPass*) scene_pass);

        // render_pass = (RenderPass*) scene_pass;

    } else if (type == DEPTH_PASS) {
        DepthPass* depth_pass = new DepthPass(
            name, _scene_passes.size() + _index, _win, _camera, _has_srgb, _has_alpha);

        // setup camera which which captures scene
        // and renders into FBO using default camera lens
        NodePath cam = depth_pass->get_camera();
        ((Camera*) cam.node())->set_scene(get_scene());
        ((Camera*) cam.node())->set_camera_mask(mask);

        _scene_passes.push_back((RenderPass*) depth_pass);

        // render_pass = (RenderPass*) depth_pass;

    } else {  // POST_PASS
        // get plane from previous render pass
        NodePath prev_plane;
        if (_post_passes.size() > 0)
            prev_plane = _post_passes.back()->get_result_card();
        else
            prev_plane = _scene_passes.back()->get_result_card();

        PostPass* post_pass = new PostPass(
            name, _scene_passes.size() + _post_passes.size() + _index, _win,
            _camera2d, _has_srgb, _has_alpha, prev_plane);

        // pass textures from the scene passes to the current render pass
        for (unsigned int i = 0; i < _scene_passes.size(); i++) {
            RenderPass* scene_pass = _scene_passes[i];
            for (unsigned int j = 0; j < scene_pass->get_num_textures(); j++) {
                PointerTo<Texture> t = scene_pass->get_texture(j);
                post_pass->get_source_card().set_shader_input(ShaderInput(t->get_name(), t));
            }
        }

        // pass textures from the previous post pass to the current render pass
        RenderPass* prev_pass = NULL;
        if (_post_passes.size()) {
            prev_pass = _post_passes.back();
        } else if (_scene_passes.size()) {
            prev_pass = _scene_passes.back();
        }
        if (prev_pass != NULL) {
            for (unsigned int j = 0; j < prev_pass->get_num_textures() && j < 1; j++) {
                PointerTo<Texture> t = prev_pass->get_texture(j);
                post_pass->get_source_card().set_shader_input(
                    ShaderInput(std::string("prev_color"), t));
            }
        }

        _post_passes.push_back((RenderPass*) post_pass);

        if (shader != nullptr)
            post_pass->get_source_card().set_shader(shader, 100);

        // setup projection camera which captures plane from previous render pass
        // and renders into FBO
        NodePath cam = post_pass->get_camera();
        ((Camera*) cam.node())->set_scene(prev_plane);

        // render_pass = (RenderPass*) post_pass;
    }
}

NodePath RenderPipeline::get_camera(char* name) {
    for (unsigned int i = 0; i < _scene_passes.size(); i++) {
        if (strcmp(_scene_passes[i]->get_name(), name) == 0) {
            return _scene_passes[i]->get_camera();
        }
    }
    for (unsigned int i = 0; i < _post_passes.size(); i++) {
        if (strcmp(_post_passes[i]->get_name(), name) == 0) {
            return _post_passes[i]->get_camera();
        }
    }
    return NodePath::not_found();
}

NodePath RenderPipeline::get_source_card(char* name) {
    for (unsigned int i = 0; i < _scene_passes.size(); i++) {
        if (strcmp(_scene_passes[i]->get_name(), name) == 0) {
            return _scene_passes[i]->get_source_card();
        }
    }
    for (unsigned int i = 0; i < _post_passes.size(); i++) {
        if (strcmp(_post_passes[i]->get_name(), name) == 0) {
            return _post_passes[i]->get_source_card();
        }
    }
    return NodePath::not_found();
}

NodePath RenderPipeline::get_result_card(char* name) {
    for (unsigned int i = 0; i < _scene_passes.size(); i++) {
        if (strcmp(_scene_passes[i]->get_name(), name) == 0) {
            return _scene_passes[i]->get_result_card();
        }
    }
    for (unsigned int i = 0; i < _post_passes.size(); i++) {
        if (strcmp(_post_passes[i]->get_name(), name) == 0) {
            return _post_passes[i]->get_result_card();
        }
    }
    return NodePath::not_found();
}

PointerTo<Texture> RenderPipeline::get_texture(char* name, unsigned int j) {
    for (unsigned int i = 0; i < _scene_passes.size(); i++) {
        if (strcmp(_scene_passes[i]->get_name(), name) == 0) {
            return _scene_passes[i]->get_texture(j);
        }
    }
    for (unsigned int i = 0; i < _post_passes.size(); i++) {
        if (strcmp(_post_passes[i]->get_name(), name) == 0) {
            return _post_passes[i]->get_texture(j);
        }
    }
    return NULL;
}

void RenderPipeline::update() {
    if (_win_w != _win->get_x_size() || _win_h != _win->get_y_size()) {
        _win_w = _win->get_x_size();
        _win_h = _win->get_y_size();
        _configure();
        for (unsigned int i = 0; i < _scene_passes.size(); i++) {
            _scene_passes[i]->reload_shader();
        }
        for (unsigned int i = 0; i < _post_passes.size(); i++) {
            _post_passes[i]->reload_shader();
        }
    }

    LightingPipeline::update();
}
