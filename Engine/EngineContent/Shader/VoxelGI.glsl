[vertex]
out vec3 pos;
void main() {
    pos = aPos;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
in vec3 pos;
uniform sampler3D voxelData;
void main() {
    vec3 texCoord = vec3(pos.x,pos.y,0.0);
    vec3 c1 = texture(voxelData,texCoord).rgb*2000;
    FragColor = vec4(c1,1.0);
}