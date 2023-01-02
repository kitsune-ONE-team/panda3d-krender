#include "camera.h"
#include "displayRegion.h"
#include "frameBufferProperties.h"
#include "geomEnums.h"
#include "renderState.h"
#include "texture.h"

#include "krender/core/lighting_pipeline.h"


TypeHandle LightingPipeline::_type_handle;

LightingPipeline::LightingPipeline(GraphicsWindow* window, NodePath scene, NodePath camera) {
    _win = window;

    _create_shadowmap(false);
    _create_shadow_manager(scene, camera);
    _create_queue();
    _create_light_manager();
}

// LightingPipeline::~LightingPipeline() {
//     free(_light_data);
// }

void LightingPipeline::_create_shadowmap(bool depth2color) {
    _atlas_size = 256;
    _is_enabled = 1;
    unsigned int shadow_size = 512;

    // each point light containts 6 shadow sources (+X, -X, +Y, -Y, +Z, -Z)
    // defining shadow sources rows/columns
    // shadow sources are placed in a matrix:
    // rows * columns -> ss_rc * ss_rc
    int ss_rc = ceil(sqrt(MAX_LIGHTS * 6));
    while (ss_rc * shadow_size > _atlas_size)
        _atlas_size *= 2;  // double atlas size

    FrameBufferProperties* fbp = new FrameBufferProperties();
    if (depth2color) {
        fbp->set_float_color(true);
        fbp->set_rgba_bits(32, 32, 32, 8);
        fbp->set_srgb_color(false);
    } else {
        fbp->set_depth_bits(32);
        fbp->set_float_depth(true);
    }

    _shadowmap_tex = new Texture("lighting_pipeline.shadowmap");
    if (_win->get_gsg()->get_supports_shadow_filter()) {
        _shadowmap_tex->set_minfilter(SamplerState::FT_shadow);
        _shadowmap_tex->set_magfilter(SamplerState::FT_shadow);
    }

    _shadowmap_fbo = _win->make_texture_buffer(
        "lighting_pipeline",
        _atlas_size, _atlas_size, _shadowmap_tex, false, fbp);
    _shadowmap_fbo->disable_clears();
    _shadowmap_fbo->get_overlay_display_region()->disable_clears();
    _shadowmap_fbo->get_overlay_display_region()->set_active(false);

    // remove default FBO's display regions
    _shadowmap_fbo->remove_all_display_regions();

    // also deactivate undeletable
    for (unsigned int i = 0; i < _shadowmap_fbo->get_num_display_regions(); i++) {
        DisplayRegion* region = _shadowmap_fbo->get_display_region(i);
        region->disable_clears();
        region->set_active(false);
    }
}

void LightingPipeline::_create_shadow_manager(NodePath scene, NodePath camera) {
    _tag_state_manager = new TagStateManager(camera);

    // shadow sources/atlas manager
    _shadow_manager = new ShadowManager();
    _shadow_manager->set_max_updates(MAX_UPDATES);
    _shadow_manager->set_scene(scene);
    _shadow_manager->set_tag_state_manager(_tag_state_manager);
    _shadow_manager->set_atlas_size(_atlas_size);
    _shadow_manager->set_atlas_graphics_output(_shadowmap_fbo);
    _shadow_manager->init();

    // context = {
    //     'DEPTH2COLOR': 1 if DEPTH2COLOR else 0,
    // }
    // shader = shared.make_shader('shadow', vert_context=context, frag_context=context);
    ConstPointerTo<RenderState> state = RenderState::make_empty();
    // state = state.set_attrib(ShaderAttrib.make(shader, 200), 200);

    // reconfiture FBO's display regions
    // which was created by ShadowManager
    for (unsigned int i = 0; i < _shadowmap_fbo->get_num_display_regions(); i++) {
        DisplayRegion* region = _shadowmap_fbo->get_display_region(i);
        region->disable_clears();
        region->set_active(false);

        region->set_clear_depth_active(true);
        region->set_clear_depth(1.0);

        // reconfigure shadow caster/source camera
        // which was configured while registering camera with a TagStateManager
        NodePath camera = region->get_camera();
        if (!camera.is_empty()) {
            ((Camera*) camera.node())->set_initial_state(state);
            // camera.node()->set_initial_state(state);
            // camera.node().set_camera_mask(CAMERA_MASK_SHADOW);
        }
    }
}

void LightingPipeline::_create_queue() {
    // GPU command list queue
    _gpu_command_list = new GPUCommandList();

    // GPU command data texture
    _gpu_command_data = new Texture("GPU Command Data");
    _gpu_command_data->setup_buffer_texture(
        GPU_COMMAND_SIZE * GPU_COMMAND_LIST_LIMIT,
        Texture::T_float,
        Texture::F_r32,
        GeomEnums::UH_static);
}

void LightingPipeline::_create_light_manager() {
    _light_manager = new InternalLightManager();
    _light_manager->set_shadow_update_distance(1000);
    _light_manager->set_command_list(_gpu_command_list);
    _light_manager->set_shadow_manager(_shadow_manager);

    _light_data = (LightData*) malloc(sizeof(LightData*));
    _light_data_tex = new Texture("Light Data");
    _light_data_tex->setup_buffer_texture(
        sizeof(_light_data),
        Texture::T_float,
        Texture::F_rgba32,
        GeomEnums::UH_static);
}

void LightingPipeline::_cmd_store_light(unsigned char* gpu_command_data) {
    LightDef* light = (LightDef*) malloc(sizeof(LightDef*));
    // read light
    memcpy(light, gpu_command_data, sizeof(light));
    // store light info
    memcpy(_light_data->contents.lights[light->packed.slot].data,
           light->packed.data, sizeof(light->packed.data));
}

void LightingPipeline::_cmd_remove_light(unsigned char* gpu_command_data) {
    int32_t slot;
    // read light slot
    memcpy(&slot, gpu_command_data, sizeof(&slot));
    // clear light info
    memset(_light_data->contents.lights[slot].data,
           0, sizeof(LightInfo*));
}

void LightingPipeline::_cmd_store_source(unsigned char* gpu_command_data) {
    ShadowSourceDef* ss = (ShadowSourceDef*) malloc(sizeof(ShadowSourceDef*));
    // read shadow source
    memcpy(ss, gpu_command_data, sizeof(ss));
    // store shadow source info
    memcpy(_light_data->contents.shadow_sources[ss->packed.slot].data,
           ss->packed.data, sizeof(ss->packed.data));
}

void LightingPipeline::_cmd_remove_sources(unsigned char* gpu_command_data) {
    int32_t slot0;
    int32_t count;
    // read first shadow source slot and count
    memcpy(&slot0, gpu_command_data, sizeof(&slot0));
    memcpy(&count, gpu_command_data + 4, sizeof(&count));
    // clear shadow sources info
    for (int i = 0; i <= count; i++) {
        memset(_light_data->contents.shadow_sources[slot0 + i].data,
               0, sizeof(ShadowSourceInfo*));
    }
}

void LightingPipeline::_update() {
    // cam_pos = builtins.base.camera.get_pos(builtins.base.render);
    LPoint3 cam_pos(10000, 10000, 10000);

    if (_is_enabled)
        _light_manager->set_camera_pos(cam_pos);
    else
        _light_manager->set_camera_pos(LPoint3(10000, 10000, 10000));

    _light_manager->update();
    _shadow_manager->update();

    PTA_uchar gpu_command_data = _gpu_command_data->modify_ram_image();
    int num_commands = _gpu_command_list->write_commands_to(gpu_command_data, GPU_COMMAND_LIST_LIMIT);
    if (!num_commands)
        return;

    for (int i = 0; i < num_commands; i++) {
        int index = i * GPU_COMMAND_SIZE * R32;
        int32_t command;
        memcpy(&command, gpu_command_data.p()+index, sizeof(&command));
        index++;

        switch (command) {
        default:
            break;
        case 1:  // CMD_store_light
            _cmd_store_light(gpu_command_data.p()+index);
            break;
        case 2:  // CMD_remove_light
            _cmd_remove_light(gpu_command_data.p()+index);
            break;
        case 3:  // CMD_store_source
            _cmd_store_source(gpu_command_data.p()+index);
            break;
        case 4:  // CMD_remove_sources
            _cmd_remove_sources(gpu_command_data.p()+index);
            break;
        }
    }

    PTA_uchar light_data = _light_data_tex->modify_ram_image();
    memcpy(light_data.p(), _light_data->data, sizeof(_light_data->data));
}

void LightingPipeline::add_light(PT(RPLight) light) {
    _light_manager->add_light(light);
}

void LightingPipeline::remove_light(PT(RPLight) light) {
    _light_manager->remove_light(light);
}
