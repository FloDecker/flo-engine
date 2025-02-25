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
uniform sampler2D light_map;



float kernel[9] = float[](
1.0 / 16, 2.0 / 16, 1.0 / 16,
2.0 / 16, 4.0 / 16, 2.0 / 16,
1.0 / 16, 2.0 / 16, 1.0 / 16
);

vec3 get_distance_blur(float distance) {


    float offset = 1.0 / distance;

    vec2 offsets[9] = vec2[](
    vec2(-offset,  offset),
    vec2( 0.0f,    offset),
    vec2( offset,  offset),
    vec2(-offset,  0.0f),
    vec2( 0.0f,    0.0f),
    vec2( offset,  0.0f),
    vec2(-offset, -offset),
    vec2( 0.0f,   -offset),
    vec2( offset, -offset)
    );

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(color_framebuffer, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
        col += sampleTex[i] * kernel[i];
    }
    return col;
}

void main()
{
    float d = texture(dpeth_framebuffer, TexCoords).x;
    //vec3 color = vec3(texture(color_framebuffer, TexCoords));
    //vec3 color = get_distance_blur(100);
    vec3 color = vec3(texture(color_framebuffer, TexCoords));
    vec3 light_map_sampled = vec3(texture(light_map, TexCoords*2.0));

    //color = vec3(d);
   
    //FragColor = vec4(vec3(color),1.0);
    if(TexCoords.x < 0.5  && TexCoords.y < 0.5){
        FragColor = vec4(light_map_sampled,1.0);
    }else {
        FragColor = vec4(vec3(color),1.0);
    }
    //FragColor = vec4(0.4);
}