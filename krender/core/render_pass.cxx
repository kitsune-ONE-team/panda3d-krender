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


RenderPass::RenderPass(char* name, unsigned int index, GraphicsWindow* win, NodePath cam) {
    _name = name;
    _index = index;

    _fbo = _make_fbo(win);
    _make_textures();

    char* cam_name = (char*) malloc((strlen(name) + strlen("_camera")) * sizeof(char));
    sprintf(cam_name, "%s_camera", name);

    if (_index == 0) {  // initial render pass
        _cam = _make_camera(_fbo, cam_name, cam);
    } else {  // other render passes
        _cam = _make_camera2d(_fbo, cam_name, cam);
    }
}

GraphicsOutput* RenderPass::get_fbo() {
    return _fbo;
}

NodePath RenderPass::get_camera() {
    return _cam;
}

Texture* RenderPass::get_texture(unsigned int i) {
    return _tex.at(i);
}

unsigned int RenderPass::get_num_textures() {
    return _tex.size();
}

GraphicsOutput* RenderPass::_make_fbo(GraphicsWindow* win) {
    WindowProperties props = win->get_properties();
    int w = props.get_x_size();
    int h = props.get_y_size();

    FrameBufferProperties* fbp = new FrameBufferProperties();
    fbp->set_rgba_bits(1, 1, 1, 1);
    if (_index == 0) {  // initial render pass
        fbp->set_depth_bits(1);
        fbp->set_aux_rgba(1);
    }

    char* tex_name = (char*) malloc((strlen(_name) + strlen("_color")) * sizeof(char));
    sprintf(tex_name, "%s_color", _name);
    Texture* t = new Texture(tex_name);

    GraphicsOutput* fbo = win->make_texture_buffer(_name, w, h, t, false, fbp);
    fbo->set_sort(_index - 10);
    fbo->set_clear_color(LVecBase4(0, 0, 0, 1));
    // fbo->clear_render_textures();
    _tex.push_back(t);

    return fbo;
}

void RenderPass::_make_textures() {
    char* tex_name;
    Texture* t;

    if (_index == 0)  // initial render pass
        return;

    tex_name = (char*) malloc((strlen(_name) + strlen("_depth")) * sizeof(char));
    sprintf(tex_name, "%s_depth", _name);
    t = new Texture(tex_name);
    _fbo->add_render_texture(
        t,
        GraphicsOutput::RTM_bind_or_copy,
        GraphicsOutput::RTP_depth);
    _tex.push_back(t);

    tex_name = (char*) malloc((strlen(_name) + strlen("_emissive")) * sizeof(char));
    sprintf(tex_name, "%s_emissive", _name);
    t = new Texture(tex_name);
    _fbo->add_render_texture(
        t,
        GraphicsOutput::RTM_bind_or_copy,
        GraphicsOutput::RTP_aux_rgba_0);
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
