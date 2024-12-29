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


vec3 sampleColorRange(float x, vec3 color0, vec3 color1, vec3 color2,
float pos0, float pos1, float pos2) {
    // Ensure positions are sorted and x is within [0, 1]
    x = clamp(x, 0.0, 1.0);

    vec3 startColor;
    vec3 endColor;
    float startPos;
    float endPos;

    // Determine which segment x lies in
    if (x < pos1) {
        startColor = color0;
        endColor = color1;
        startPos = pos0;
        endPos = pos1;
    } else if (x < pos2) {
        startColor = color1;
        endColor = color2;
        startPos = pos1;
        endPos = pos2;
    } else {
        // Fallback in case x is slightly out of bounds due to precision
        return color2;
    }

    // Normalize x within the segment and interpolate
    float t = (x - startPos) / (endPos - startPos);
    return mix(startColor, endColor, t);
}


in vec3 pos_ws;
void main() {

    float heigt = (pos_ws.y/(SKY_SPHERE_SCALE*2)+0.5);  
    float f_0 = smoothstep(TRANSITION_0,TRANSITION_1,heigt);
    float f_1 = smoothstep(TRANSITION_2,TRANSITION_3,heigt);
    vec3 color_mix = sampleColorRange(heigt,u_ambient_light_colors[0] , u_ambient_light_colors[1] , u_ambient_light_colors[2],
    u_ambient_light_colors_sample_positions[0], u_ambient_light_colors_sample_positions[1], u_ambient_light_colors_sample_positions[2]);
    FragColor = vec4(color_mix,1.0);
}