[vertex]

void main() {
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
uniform vec3 color;
void main() {
  
    FragColor = vec4(color,1.0f);
}