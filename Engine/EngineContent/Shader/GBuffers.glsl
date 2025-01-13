[vertex]
out vec3 world_pos;
void main() {
    world_pos = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
in vec3 world_pos;

void main() {
    FragColor = vec4(world_pos,1.0);
}