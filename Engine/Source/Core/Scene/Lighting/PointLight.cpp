#include "PointLight.h"

PointLight::PointLight(Object3D *parent): light(parent)
{
    add_tag("ENGINE_LIGHT_POINT");
}
