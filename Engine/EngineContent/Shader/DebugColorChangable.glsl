[vertex]

void main_u() {
}

[fragment]

uniform vec3 color;
void main_u() {
    gAlbedoSpec = vec4(color, 1.0);
}