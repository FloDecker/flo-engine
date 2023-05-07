[vertex]

const vec3 ligtPos = 30;

out vec3 cameraPosWs;
out vec3 vertexPosWS;
void main() {
    cameraPosWs = cameraPosWS;
    vertexPosWS = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
in vec3 cameraPosWs;
in vec3 vertexPosWS;
void main() {
    FragColor = vec4(distance(cameraPosWs,vertexPosWS)*0.1);
}