[vertex]
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aUv;
}

[fragment]
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D color_framebuffer;
uniform sampler2D dpeth_framebuffer;

void main()
{
    float d = texture(dpeth_framebuffer, TexCoords).x;
    vec3 color = vec3(texture(color_framebuffer, TexCoords));
    if(TexCoords.x < 0.5 ){
        FragColor = vec4(vec3(color),1.0);
    }else {
        FragColor = vec4(vec3(pow(d,100)),1.0);
    }
    //FragColor = vec4(0.4);
}