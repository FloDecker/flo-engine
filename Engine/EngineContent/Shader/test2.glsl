[vertex]
out  vec3 posWS;
flat out vec3 vertexColor;
flat out vec3 normalWS;
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    posWS = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
    vertexColor = vec3(rand(posWS.xy*200.0),rand(posWS.yz*200.+141.0),rand(posWS.xz*200+12.0));
}

[fragment]
in vec3 posWS;
flat in vec3 vertexColor;
void main() {
    FragColor = vec4(vec3(vertexColor),1.0f);
    FragColor = vec4(vec3(1.0f),1.0f);
}