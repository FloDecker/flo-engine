[vertex]

void main_u() {
}

[fragment]
void main_u() {
    gAlbedoSpec = vec4(1,0,0,1);
    gRenderFlags = 1u;

    gl_FragDepth = 0.0;
}