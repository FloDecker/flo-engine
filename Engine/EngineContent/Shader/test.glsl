#version 430 core
#define FORWARD
out vec4 FragColor;

#ifdef DEFAULT_VERTEX_SHADER
in vec3 pos_ws;
in vec3 normal_ws;
void main(){
    main_u();
}
#endif

#ifndef DEFAULT_VERTEX_SHADER
void main(){
    main_u();
}
#endif
