[vertex]

void main() {
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
in vec4 gl_FragCoord;
void main() {
    vec3 col = vec3(float(mod(gl_FragCoord.y, 2) < 1) * float(mod(gl_FragCoord.x, 2) < 1) );
    
    FragColor = vec4(col*vec3(255,0,220),1.0);
}