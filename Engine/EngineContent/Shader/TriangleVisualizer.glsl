[vertex]
flat out vec3 random_color;


float random(vec2 st)
{
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}
void main() {
    vec3 pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    random_color = vec3(random(pos_ws.xy),random(pos_ws.xz),random(pos_ws.yz));

    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
flat in vec3 random_color;
void main() {
    FragColor = vec4(random_color,1.0);

}