#ifndef PANDA_INSTANCE_H
#define PANDA_INSTANCE_H

#include "nodePath.h"
#include "pandaNode.h"

#define MAX_INSTANCES 1000
#define FLOAT_SIZE 4
#define MAT4_HEIGHT 4
#define MAT4_SIZE (MAT4_WIDTH * MAT4_HEIGHT * FLOAT_SIZE)
#define MAT4_WIDTH 4
#define RGBA_CHANNEL_COUNT 4
#define RGBA_MAT4_SIZE ((MAT4_WIDTH * MAT4_HEIGHT) / RGBA_CHANNEL_COUNT)


typedef union LMatrix4Array {
    LMatrix4 matrices[MAX_INSTANCES];
    unsigned char data[MAT4_SIZE * MAX_INSTANCES];
} LMatrix4Array;


class EXPORT_CLASS InstanceNode: public PandaNode {
PUBLISHED:
    explicit InstanceNode(const char* name, unsigned int num_instances);
    void set_transform(unsigned int instance_id, LMatrix4 mat);
    void set_prev_transform(unsigned int instance_id, LMatrix4 mat);
    void set_time(unsigned int instance_id, float time);
    void set_prev_time(unsigned int instance_id, float time);
    void setup();
    void setup(NodePath np);
    void update_shader_inputs();
    void update_shader_inputs(NodePath np);

private:
    unsigned int _num_instances;
    LMatrix4Array* _instance_transform;
    LMatrix4Array* _instance_prev_transform;
    float _instance_time[MAX_INSTANCES];
    float _instance_prev_time[MAX_INSTANCES];
    PointerTo<Texture> _instance_transform_tex;
    PointerTo<Texture> _instance_prev_transform_tex;
    PointerTo<Texture> _instance_time_tex;
    PointerTo<Texture> _instance_prev_time_tex;
    static TypeHandle _type_handle;

public:
    static TypeHandle get_class_type() {
        return _type_handle;
    }
    static void init_type() {
        PandaNode::init_type();
        register_type(_type_handle, "InstanceNode", PandaNode::get_class_type());
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
