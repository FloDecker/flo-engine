[vertex]

out vec3 pos_ws;
void main() {
    pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
float SKY_SPHERE_SCALE = 512.0;
vec3 COLOR_SKY_BOTTOM = vec3(1.0,0.0,0.0);
vec3 COLOR_SKY_MIDDLE = vec3(0.0,1.0,0.0);
vec3 COLOR_SKY_TOP = vec3(0.0,0.0,1.0);

float TRANSITION_0 = 0.5;
float TRANSITION_1 = 0.7;

in vec3 pos_ws;
void main() {
    float heigt = (pos_ws.y/(SKY_SPHERE_SCALE*2)+0.5);  
    vec3 color_mix = mix(mix(COLOR_SKY_BOTTOM,COLOR_SKY_MIDDLE,heigt),COLOR_SKY_TOP,heigt);
    FragColor = vec4(color_mix,1.0);
}