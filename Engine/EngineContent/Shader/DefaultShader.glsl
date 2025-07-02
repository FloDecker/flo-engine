[vertex]

void main_u() {
}

[fragment]
void main_u() {
    vec3 col = vec3(float(mod(gl_FragCoord.y*0.5, 2) < 1) * float(mod(gl_FragCoord.x*0.5, 2) < 1));
    gAlbedoSpec = vec4(col*vec3(255, 0, 220), 1.0);
}