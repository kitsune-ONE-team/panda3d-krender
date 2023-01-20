#ifndef CORE_RENDER_PIPELINE_H
#define CORE_RENDER_PIPELINE_H

#include <vector>

#include "graphicsWindow.h"
#include "nodePath.h"
#include "pandabase.h"
#include "typedWritableReferenceCount.h"

#include "krender/core/render_pass.h"
#include "krender/core/lighting_pipeline.h"


class EXPORT_CLASS RenderPipeline: public LightingPipeline {
PUBLISHED:
    RenderPipeline(
            GraphicsWindow* window, NodePath render2d, NodePath camera, NodePath camera2d,
            unsigned int shadow_size=512,
            bool has_srgb=false, bool has_alpha=false): LightingPipeline(window, camera, shadow_size) {
        _camera2d = camera2d;
        _render2d = render2d;
        _has_srgb = has_srgb;
        _has_alpha = has_alpha;
    };
    void add_render_pass(char* name, Shader* shader=NULL);

private:
    NodePath _camera2d;
    NodePath _render2d;
    bool _has_srgb;
    bool _has_alpha;

    int _index;
    std::vector<RenderPass*> _passes;
    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        LightingPipeline::init_type();
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
