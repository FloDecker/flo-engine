[vertex]

void main() {
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]

void main() {
    FragColor = vec4(1.0f);
}