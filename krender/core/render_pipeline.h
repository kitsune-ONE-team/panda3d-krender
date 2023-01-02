#ifndef CORE_RENDER_PIPELINE_H
#define CORE_RENDER_PIPELINE_H

#include <vector>

#include "pandabase.h"
#include "graphicsWindow.h"
#include "nodePath.h"
#include "typedWritableReferenceCount.h"

#include "krender/core/render_pass.h"


class EXPORT_CLASS RenderPipeline: public TypedWritableReferenceCount {
PUBLISHED:
    RenderPipeline(
        GraphicsWindow* window, NodePath render2d, NodePath camera, NodePath camera2d,
        bool has_srgb=false, bool has_alpha=false);

    void add_render_pass(char* name, Shader* shader=NULL);

private:
    GraphicsWindow* _win;
    NodePath _camera;
    NodePath _camera2d;
    NodePath _render2d;
    bool _has_srgb;
    bool _has_alpha;

    int _index;
    NodePath _scene;
    std::vector<RenderPass*> _passes;
    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedWritableReferenceCount::init_type();
        register_type(
            _type_handle, "RenderPipeline",
            TypedWritableReferenceCount::get_class_type());
    }
    virtual TypeHandle get_type() const {
        return get_class_type();
    }
    virtual TypeHandle force_init_type() {
        init_type();
        return get_class_type();
    }
};

#endif
