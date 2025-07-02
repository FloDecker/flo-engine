[vertex]
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 mMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * mMatrix * vec4(aPos, 1.0);
}

[fragment]

void main()
{
    // gl_FragDepth = gl_FragCoord.z;
}  