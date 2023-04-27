[vertex]

out vec3 posWS;
out vec3 normal;
out vec2 texCoord;
void main() {
    texCoord = aUV;
    normal = aNormal;
    posWS =( mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace; 
}

[fragment]
in vec3 posWS;
in vec3 normal;
in vec2 texCoord;
uniform sampler2D textureBase;
uniform sampler2D textureNormal;

void main() {
    //FragColor = vec4(normal + abs(normal)*1.05, 1.0f);
    //FragColor = vec4(texCoord,0.0f, 1.0f);
    vec3 c1 = texture(textureBase,texCoord).rgb;
    vec3 c2 = texture(textureNormal,texCoord).rgb;
    
    FragColor = vec4(c2,1.0);
}