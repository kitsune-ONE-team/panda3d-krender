#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <type_traits>

#include "displayRegion.h"
#include "frameBufferProperties.h"
#include "graphicsWindow.h"
#include "pandaNode.h"
#include "texture.h"
#include "transformState.h"
#include "virtualFileSystem.h"
#include "windowProperties.h"

#include "krender/core/render_pass.h"


RenderPass::RenderPass(
        char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
        bool has_srgb, bool has_alpha, float sx, float sy, NodePath card) {
    _name = name;
    _index = index;
    _source_card = card;
    _scale_x = sx;
    _scale_y = sy;
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

PointerTo<Texture> RenderPass::get_texture(unsigned int i) {
    return _tex[i];
}

unsigned int RenderPass::get_num_textures() {
    return _tex.size();
}

void RenderPass::reload_shader() {
    if (get_source_card().is_empty())
        return;

    const Shader* shaderc = get_source_card().get_shader();
    if (shaderc == nullptr)
        return;

    Filename vert_path = shaderc->get_filename(Shader::ST_vertex).get_fullpath();
    Filename frag_path = shaderc->get_filename(Shader::ST_fragment).get_fullpath();

    Shader* shader = Shader::load(Shader::SL_GLSL, vert_path, frag_path);
    if (shader == nullptr)
        return;

    shader->set_filename(Shader::ST_vertex, vert_path);
    shader->set_filename(Shader::ST_fragment, frag_path);

    get_source_card().clear_shader();
    get_source_card().set_shader(shader, 100);
}

PointerTo<GraphicsOutput> RenderPass::_make_fbo(
        PointerTo<GraphicsWindow> win, bool has_srgb, bool has_alpha,
        bool has_color, bool has_depth,
        unsigned short num_aux_textures, unsigned short sort) {
    FrameBufferProperties* fbp = new FrameBufferProperties();
    if (has_color)
        fbp->set_rgba_bits(1, 1, 1, 1);
    else
        fbp->set_rgba_bits(0, 0, 0, 0);

    if (has_srgb)
        fbp->set_srgb_color(true);

    if (has_depth) {
        fbp->set_depth_bits(32);
        fbp->set_float_depth(true);
    }

    fbp->set_aux_rgba(num_aux_textures);

    int fbo_width = _scale_x == 1 ? 0 : win->get_x_size() * _scale_x;
    int fbo_height = _scale_y == 1 ? 0 : win->get_y_size() * _scale_y;
    PointerTo<GraphicsOutput> fbo = win->make_texture_buffer(
        _name, fbo_width, fbo_height, nullptr, false, fbp);
    fbo->clear_render_textures();
    fbo->set_sort(sort);
    if (has_alpha)
        fbo->set_clear_color(LVecBase4(0, 0, 0, 0));
    else
        fbo->set_clear_color(LVecBase4(0, 0, 0, 1));

    char* card_name = (char*) malloc((strlen(_name) + strlen("_card") + 1) * sizeof(char));
    sprintf(card_name, "%s_card", _name);

    _result_card = fbo->get_texture_card();
    _result_card.set_name(card_name);

    return fbo;
}

void RenderPass::_make_textures(
        bool has_color, bool has_depth, unsigned short num_aux_textures) {
    if (has_color) {
        char* tex_name = (char*) malloc((strlen(_name) + strlen("_color") + 1) * sizeof(char));
        sprintf(tex_name, "%s_color", _name);

        PointerTo<Texture> t = new Texture(tex_name);
        t->set_format(Texture::Format::F_srgb_alpha);
        t->set_wrap_u(SamplerState::WM_clamp);
        t->set_wrap_v(SamplerState::WM_clamp);
        t->set_magfilter(SamplerState::FilterType::FT_linear);
        t->set_minfilter(SamplerState::FilterType::FT_linear);
        _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_color);
        _tex.push_back(t);
    }

    if (has_depth) {
        char* tex_name = (char*) malloc((strlen(_name) + strlen("_depth") + 1) * sizeof(char));
        sprintf(tex_name, "%s_depth", _name);

        PointerTo<Texture> t = new Texture(tex_name);
        t->set_format(Texture::Format::F_depth_component);
        t->set_wrap_u(SamplerState::WM_clamp);
        t->set_wrap_v(SamplerState::WM_clamp);
        t->set_magfilter(SamplerState::FilterType::FT_linear);
        t->set_minfilter(SamplerState::FilterType::FT_linear);
        _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_depth);
        _tex.push_back(t);
    }

    if (num_aux_textures >= 1) {
        char* tex_name = (char*) malloc((strlen(_name) + strlen("_emissive") + 1) * sizeof(char));
        sprintf(tex_name, "%s_emissive", _name);

        PointerTo<Texture> t = new Texture(tex_name);
        t->set_format(Texture::Format::F_srgb_alpha);
        t->set_wrap_u(SamplerState::WM_clamp);
        t->set_wrap_v(SamplerState::WM_clamp);
        t->set_magfilter(SamplerState::FilterType::FT_linear);
        t->set_minfilter(SamplerState::FilterType::FT_linear);
        _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_aux_rgba_0);
        _tex.push_back(t);
    }

    if (num_aux_textures >= 2) {
        char* tex_name = (char*) malloc((strlen(_name) + strlen("_normal") + 1) * sizeof(char));
        sprintf(tex_name, "%s_normal", _name);

        PointerTo<Texture> t = new Texture(tex_name);
        t->set_format(Texture::Format::F_srgb_alpha);
        t->set_wrap_u(SamplerState::WM_clamp);
        t->set_wrap_v(SamplerState::WM_clamp);
        t->set_magfilter(SamplerState::FilterType::FT_linear);
        t->set_minfilter(SamplerState::FilterType::FT_linear);
        _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_aux_rgba_1);
        _tex.push_back(t);
    }

    if (num_aux_textures >= 3) {
        char* tex_name = (char*) malloc((strlen(_name) + strlen("_selector") + 1) * sizeof(char));
        sprintf(tex_name, "%s_selector", _name);

        PointerTo<Texture> t = new Texture(tex_name);
        t->set_format(Texture::Format::F_srgb_alpha);
        t->set_wrap_u(SamplerState::WM_clamp);
        t->set_wrap_v(SamplerState::WM_clamp);
        t->set_magfilter(SamplerState::FilterType::FT_linear);
        t->set_minfilter(SamplerState::FilterType::FT_linear);
        _fbo->add_render_texture(t, GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_aux_rgba_2);
        _tex.push_back(t);
    }
}
