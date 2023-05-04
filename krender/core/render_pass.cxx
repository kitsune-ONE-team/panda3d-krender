#include <stdlib.h>
#include <string.h>

#include "displayRegion.h"
#include "frameBufferProperties.h"
#include "graphicsWindow.h"
#include "lens.h"
#include "orthographicLens.h"
#include "pandaNode.h"
#include "texture.h"
#include "transformState.h"
#include "windowProperties.h"

#include "krender/core/render_pass.h"


RenderPass::RenderPass(char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
                       bool has_srgb, bool has_alpha, NodePath card) {
    _name = name;
    _index = index;
    _source_card = card;

    _fbo = _make_fbo(win, has_srgb, has_alpha);
    _make_textures();

    char* cam_name = (char*) malloc((strlen(name) + strlen("_camera")) * sizeof(char));
    sprintf(cam_name, "%s_camera", name);

    if (_index == 0) {  // initial render pass
        _cam = _make_camera(_fbo, cam_name, cam);
    } else {  // other render passes
        _cam = _make_camera2d(_fbo, cam_name, cam);
    }
}

char* RenderPass::get_name() {
    return _name;
}

NodePath RenderPass::get_camera() {
    return _cam;
}

NodePath RenderPass::get_source_card() {
    return _source_card;
}

NodePath RenderPass::get_result_card() {
    return _result_card;
}

Texture* RenderPass::get_texture(unsigned int i) {
    return _tex.at(i);
}

unsigned int RenderPass::get_num_textures() {
    return _tex.size();
}

void RenderPass::reload_shader() {
    if (_source_card.is_empty())
        return;

    const Shader* shaderc = _source_card.get_shader();
    if (shaderc == nullptr)
        return;

    Filename vert = shaderc->get_filename(Shader::ST_vertex).get_fullpath();
    Filename frag = shaderc->get_filename(Shader::ST_fragment).get_fullpath();

    Shader* shader = Shader::load(Shader::SL_GLSL, vert, frag);
    _source_card.clear_shader();
    _source_card.set_shader(shader, 100);
}

GraphicsOutput* RenderPass::_make_fbo(GraphicsWindow* win, bool has_srgb, bool has_alpha) {
    FrameBufferProperties* fbp = new FrameBufferProperties();
    fbp->set_rgba_bits(1, 1, 1, 1);
    if (_index == 0) {  // initial render pass
        fbp->set_depth_bits(1);
        fbp->set_aux_rgba(1);
        if (has_srgb)
            fbp->set_srgb_color(true);
    }

    GraphicsOutput* fbo = win->make_texture_buffer(_name, 0, 0, nullptr, false, fbp);
    fbo->clear_render_textures();
    fbo->set_sort(_index - 10);
    if (has_alpha)
        fbo->set_clear_color(LVecBase4(0, 0, 0, 0));
    else
        fbo->set_clear_color(LVecBase4(0, 0, 0, 1));

    char* card_name = (char*) malloc((strlen(_name) + strlen("_card")) * sizeof(char));
    sprintf(card_name, "%s_card", _name);

    _result_card = fbo->get_texture_card();
    _result_card.set_name(card_name);

    return fbo;
}

void RenderPass::_make_textures() {
    char* tex_name;
    Texture* t;

    t = new Texture("color");
    t->set_wrap_u(SamplerState::WM_clamp);
    t->set_wrap_v(SamplerState::WM_clamp);
    _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_color);
    _tex.push_back(t);

    if (_index != 0)  // not initial render pass
        return;

    t = new Texture("depth");
    t->set_wrap_u(SamplerState::WM_clamp);
    t->set_wrap_v(SamplerState::WM_clamp);
    _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_depth);
    _tex.push_back(t);

    t = new Texture("emissive");
    t->set_wrap_u(SamplerState::WM_clamp);
    t->set_wrap_v(SamplerState::WM_clamp);
    _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_aux_rgba_0);
    _tex.push_back(t);
}

/*
 * Makes a new 3D camera, ShowBase's makeCamera reimplementation.
 */
NodePath RenderPass::_make_camera(GraphicsOutput* fbo, char* name, NodePath camera) {
    NodePath cam = camera.attach_new_node(new Camera(name));

    ((Camera*) cam.node())->set_lens(((Camera*) camera.node())->get_lens());

    DisplayRegion* dr = fbo->make_display_region();
    dr->set_sort(0);
    dr->set_camera(cam);

    return cam;
}

/*
 * Makes a new 2D camera, ShowBase's makeCamera2d reimplementation.
 */
NodePath RenderPass::_make_camera2d(GraphicsOutput* fbo, char* name, NodePath camera2d) {
    NodePath cam = camera2d.attach_new_node(new Camera(name));

    const int left = -1;
    const int right = 1;
    const int bottom = -1;
    const int top = 1;
    Lens* lens = new OrthographicLens();
    lens->set_film_size(right - left, top - bottom);
    lens->set_film_offset((right + left) * 0.5, (top + bottom) * 0.5);
    lens->set_near_far(-1000, 1000);
    ((Camera*) cam.node())->set_lens(lens);

    DisplayRegion* dr = fbo->make_mono_display_region();
    dr->set_clear_depth_active(1);
    dr->set_incomplete_render(false);
    dr->set_sort(10);
    dr->set_camera(cam);

    return cam;
}
