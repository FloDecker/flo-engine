#include "PointLight.h"

PointLight::PointLight(Object3D *parent): Object3D(parent)
{
    add_tag("ENGINE_LIGHT_POINT");
}
