#include "PointLight.h"

PointLight::PointLight(GlobalContext* global_context): Object3D(global_context)
{
    add_tag("ENGINE_LIGHT_POINT");
}
