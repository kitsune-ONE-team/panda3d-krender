#ifndef CORE_RENDER_PASS_H
#define CORE_RENDER_PASS_H

#include <vector>

#include "graphicsOutput.h"
#include "graphicsWindow.h"
#include "nodePath.h"
#include "texture.h"


class RenderPass {
public:
    RenderPass(char* name, unsigned int index, GraphicsWindow* win, NodePath cam,
               bool has_srgb=false, bool has_alpha=false, NodePath card=NodePath::not_found());
    char* get_name();
    NodePath get_camera();
    NodePath get_source_card();
    NodePath get_result_card();
    Texture* get_texture(unsigned int i);
    unsigned int get_num_textures();
    void reload_shader();

private:
    char* _name;
    unsigned int _index;
    GraphicsOutput* _fbo;
    std::vector<Texture*> _tex;
    NodePath _cam;
    NodePath _source_card;
    NodePath _result_card;

    GraphicsOutput* _make_fbo(GraphicsWindow* win, bool has_srgb=false, bool has_alpha=false);
    void _make_textures();
    NodePath _make_camera(GraphicsOutput* fbo, char* name, NodePath camera);
    NodePath _make_camera2d(GraphicsOutput* fbo, char* name, NodePath camera);
};

#endif
