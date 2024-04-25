[vertex]

out vec3 pos_ws;
void main() {
    pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
float SKY_SPHERE_SCALE = 512.0;
vec3 COLOR_SKY_BOTTOM = vec3(0.2,0.3,0.4);
vec3 COLOR_SKY_MIDDLE = vec3(0.5294,0.8078,0.9215);
vec3 COLOR_SKY_TOP = vec3(0.3882,0.8980,0.9490);

float TRANSITION_0 = 0.1;
float TRANSITION_1 = 0.5;
float TRANSITION_2 = 0.7;
float TRANSITION_3 = 0.95;

in vec3 pos_ws;
void main() {

    float heigt = (pos_ws.y/(SKY_SPHERE_SCALE*2)+0.5);  
    float f_0 = smoothstep(TRANSITION_0,TRANSITION_1,heigt);
    float f_1 = smoothstep(TRANSITION_2,TRANSITION_3,heigt);
    vec3 color_mix = mix(mix(COLOR_SKY_BOTTOM,COLOR_SKY_MIDDLE,f_0),COLOR_SKY_TOP,f_1);
    FragColor = vec4(color_mix,1.0);
}