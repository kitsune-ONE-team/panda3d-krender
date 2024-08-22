#include "nodePath.h"
#include "omniBoundingVolume.h"

#include "krender/core/instance.h"


TypeHandle InstanceNode::_type_handle;

InstanceNode::InstanceNode(const char* name, unsigned int num_instances): PandaNode(name) {
    _num_instances = num_instances <= MAX_INSTANCES ? num_instances : MAX_INSTANCES;

    _instance_transform = (LMatrix4Array*) malloc(sizeof(LMatrix4Array));
    _instance_prev_transform = (LMatrix4Array*) malloc(sizeof(LMatrix4Array));

    _instance_transform_tex = new Texture();
    _instance_transform_tex->setup_buffer_texture(
        RGBA_MAT4_SIZE * MAX_INSTANCES, Texture::T_float,
        Texture::F_rgba32, GeomEnums::UH_static);

    _instance_prev_transform_tex = new Texture();
    _instance_prev_transform_tex->setup_buffer_texture(
        RGBA_MAT4_SIZE * MAX_INSTANCES, Texture::T_float,
        Texture::F_rgba32, GeomEnums::UH_static);

    _instance_time_tex = new Texture();
    _instance_time_tex->setup_buffer_texture(
        MAX_INSTANCES * FLOAT_SIZE, Texture::T_float,
        Texture::F_rgba32, GeomEnums::UH_static);

    _instance_prev_time_tex = new Texture();
    _instance_prev_time_tex->setup_buffer_texture(
        MAX_INSTANCES * FLOAT_SIZE, Texture::T_float,
        Texture::F_rgba32, GeomEnums::UH_static);
}

void InstanceNode::set_transform(unsigned int instance_id, LMatrix4 mat) {
    _instance_transform->matrices[instance_id] = mat;
}

void InstanceNode::set_prev_transform(unsigned int instance_id, LMatrix4 mat) {
    _instance_prev_transform->matrices[instance_id] = mat;
}

void InstanceNode::set_time(unsigned int instance_id, float time) {
    _instance_time[instance_id] = time;
}

void InstanceNode::set_prev_time(unsigned int instance_id, float time) {
    _instance_prev_time[instance_id] = time;
}

void InstanceNode::setup() {
    NodePath np = NodePath::any_path(this);
    NodePathCollection nps = np.find_all_matches("**/**");
    for (int i = 0; i < nps.get_num_paths(); i++) {
        NodePath child_np = nps.get_path(i);
        setup(child_np);
    }
}

void InstanceNode::setup(NodePath np) {
    np.set_instance_count(_num_instances);
    np.node()->set_bounds(new OmniBoundingVolume());
    np.node()->set_final(true);
}

void InstanceNode::update_shader_inputs() {
    NodePath np = NodePath::any_path(this);
    NodePathCollection nps = np.find_all_matches("**/**");
    for (int i = 0; i < nps.get_num_paths(); i++) {
        NodePath child_np = nps.get_path(i);
        update_shader_inputs(child_np);
    }
}

void InstanceNode::update_shader_inputs(NodePath np) {
    PTA_uchar data = _instance_transform_tex->modify_ram_image();
    memcpy(data.p(), _instance_transform->data, sizeof(_instance_transform->data));
    np.set_shader_input("instance_transform_tex", _instance_transform_tex);

    data = _instance_prev_transform_tex->modify_ram_image();
    memcpy(data.p(), _instance_prev_transform->data, sizeof(_instance_prev_transform->data));
    np.set_shader_input("instance_prev_transform_tex", _instance_prev_transform_tex);

    data = _instance_time_tex->modify_ram_image();
    memcpy(data.p(), _instance_time, sizeof(_instance_time));
    np.set_shader_input("instance_time_tex", _instance_time_tex);

    data = _instance_prev_time_tex->modify_ram_image();
    memcpy(data.p(), _instance_prev_time, sizeof(_instance_prev_time));
    np.set_shader_input("instance_prev_time_tex", _instance_prev_time_tex);
}
