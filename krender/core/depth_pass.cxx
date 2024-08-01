#include "krender/core/depth_pass.h"


DepthPass::DepthPass(
        char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
        bool has_srgb, bool has_alpha, NodePath card):
        RenderPass(name, index, win, cam, has_srgb, has_alpha, card) {
    _fbo = _make_fbo(win, has_srgb, has_alpha, 1, _index);
    _make_textures();
    char* cam_name = (char*) malloc((strlen(name) + strlen("_camera") + 1) * sizeof(char));
    sprintf(cam_name, "%s_camera", name);
    _cam = _make_camera(_fbo, cam_name, cam);
}

PointerTo<GraphicsOutput> DepthPass::_make_fbo(
        PointerTo<GraphicsWindow> win, bool has_srgb, bool has_alpha,
        unsigned short num_textures, unsigned short sort) {
    FrameBufferProperties* fbp = new FrameBufferProperties();
    fbp->set_rgba_bits(0, 0, 0, 0);
    if (has_srgb)
        fbp->set_srgb_color(true);
    fbp->set_depth_bits(32);
    fbp->set_float_depth(true);
    fbp->set_aux_rgba(0);

    PointerTo<GraphicsOutput> fbo = win->make_texture_buffer(_name, 0, 0, nullptr, false, fbp);
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

void DepthPass::_make_textures() {
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

/*
 * Makes a new 3D camera, ShowBase's makeCamera reimplementation.
 */
NodePath DepthPass::_make_camera(PointerTo<GraphicsOutput> fbo, char* name, NodePath camera) {
    NodePath cam = camera.attach_new_node(new Camera(name));

    ((Camera*) cam.node())->set_lens(((Camera*) camera.node())->get_lens());

    DisplayRegion* dr = fbo->make_display_region();
    dr->set_sort(_index);
    dr->set_camera(cam);

    return cam;
}
