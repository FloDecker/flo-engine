[vertex]

void main_u() {
}

[fragment]
void main_u() {
    vec3 col = vec3(float(mod(gl_FragCoord.y*0.5, 2) < 1) * float(mod(gl_FragCoord.x*0.5, 2) < 1));
    gAlbedoSpec = vec4(clamp(col,vec3(0.0),vec3(1.0))*vec3(1, 0, 220.0/ 255.0), 1.0);
}