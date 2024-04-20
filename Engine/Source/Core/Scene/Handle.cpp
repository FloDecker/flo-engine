#include "Handle.h"


Handle::Handle(GlobalContext* global_context, Object3D* root): Object3D(global_context)
{
    //load handler models
    auto engine_handler_arrow_model = loadModel("EngineContent/Arrow.fbx");
    engine_handler_arrow_model->initializeVertexArrays();

    //load handler shader
    auto *handler_red = new ShaderProgram();
    handler_red->loadFromFile("EngineContent/Shader/HandlerRed.glsl");
    handler_red->compileShader();

    auto *handler_green = new ShaderProgram();
    handler_green->loadFromFile("EngineContent/Shader/HandlerGreen.glsl");
    handler_green->compileShader();

    auto *handler_blue = new ShaderProgram();
    handler_blue->loadFromFile("EngineContent/Shader/HandlerBlue.glsl");
    handler_blue->compileShader();

 
    
    auto engine_handler_arrow_mesh_x = new Mesh3D(engine_handler_arrow_model, global_context);
    engine_handler_arrow_mesh_x->materials.push_back(handler_red);
    engine_handler_arrow_mesh_x->setRotationLocalDegrees(0,90,0);
    
    auto engine_handler_arrow_mesh_y = new Mesh3D(engine_handler_arrow_model, global_context);
    engine_handler_arrow_mesh_y->materials.push_back(handler_green);
    engine_handler_arrow_mesh_y->setRotationLocalDegrees(-90,0,0);
    
    auto engine_handler_arrow_mesh_z = new Mesh3D(engine_handler_arrow_model, global_context);
    engine_handler_arrow_mesh_z->materials.push_back(handler_blue);

    addChild(engine_handler_arrow_mesh_x);
    addChild(engine_handler_arrow_mesh_y);
    addChild(engine_handler_arrow_mesh_z);

    detach();
}

void Handle::attach_to_object(Object3D* object_3d)
{
    visible = true;
    attached_ = true;
    attached_object_3d_ = object_3d;
    this->setPositionLocal(object_3d->getWorldPosition());
}

void Handle::detach()
{
    visible = false;
    attached_ = false;
    attached_object_3d_ = nullptr;
}

bool Handle::is_attached() const
{
    return attached_;
}
