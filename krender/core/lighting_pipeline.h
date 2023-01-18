#ifndef CORE_LIGHTING_PIPELINE_H
#define CORE_LIGHTING_PIPELINE_H

#include <vector>

#include "graphicsOutput.h"
#include "graphicsWindow.h"
#include "nodePath.h"
#include "pandabase.h"
#include "texture.h"
#include "typedWritableReferenceCount.h"

#ifdef CPPPARSER  // interrogate
class GPUCommandList;
class InternalLightManager;
class RPLight;
class ShadowManager;
class TagStateManager;

#else  // normal compiler
#include "gpuCommandList.h"
#include "internalLightManager.h"
#include "shadowManager.h"
#include "tagStateManager.h"
#endif

#include "krender/core/light_data.h"

#define GPU_COMMAND_LIST_LIMIT 1024
#define GPU_COMMAND_SIZE 32
#define MAX_UPDATES 1
#define R32 4
#define RGBA32 (R32 * 4)


class LightingPipeline: public TypedWritableReferenceCount {
PUBLISHED:
    NodePath get_scene();
    void update();
    void add_light(PT(RPLight) light);
    void remove_light(PT(RPLight) light);
    void prepare_scene();

public:
    LightingPipeline(GraphicsWindow* window, NodePath camera);
    Texture* get_shadowmap();
    Texture* get_light_data();

protected:
    GraphicsWindow* _win;
    NodePath _camera;

private:
    NodePath _scene;
    GraphicsOutput* _shadowmap_fbo;
    Texture* _shadowmap_tex;

    GPUCommandList* _gpu_command_list;
    Texture* _gpu_command_data;

    TagStateManager* _tag_state_manager;
    ShadowManager* _shadow_manager;
    InternalLightManager* _light_manager;
    LightData* _light_data;
    Texture* _light_data_tex;

    short _atlas_size;
    bool _is_enabled;
    static TypeHandle _type_handle;

    void _create_shadowmap(bool depth2color);
    void _create_shadow_manager(NodePath scene, NodePath camera);
    void _create_queue();
    void _create_light_manager();
    void _cmd_store_light(unsigned char* gpu_command_data);
    void _cmd_remove_light(unsigned char* gpu_command_data);
    void _cmd_store_source(unsigned char* gpu_command_data);
    void _cmd_remove_sources(unsigned char* gpu_command_data);

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        TypedWritableReferenceCount::init_type();
        register_type(
            _type_handle, "LightingPipeline",
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
