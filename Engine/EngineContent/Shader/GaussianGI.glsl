[vertex]
out vec3 cameraPosWs;
out vec3 vertexPosWs;
out vec3 normalWS;
out vec4 FragPosLightSpace;


void main() {
    cameraPosWs = cameraPosWS;
    vertexPosWs = (mMatrix * vec4(aPos, 1.0)).xyz;
    normalWS = (transpose(inverse(mMatrix)) * vec4(aNormal, 0.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
#define PI 3.14159265359
#define AREA_UNIT_SPHERE (4.0*PI)
#define FLOAT_MAX 200000.0
#define GAUSSIAN_SAMPLES 512

in vec3 cameraPosWs;
in vec3 vertexPosWs;
in vec3 normalWS;

uniform samplerBuffer surfels_texture_buffer_color_;
uniform samplerBuffer surfels_texture_buffer_normals_;
uniform samplerBuffer surfels_texture_buffer_positions_;
uniform samplerBuffer surfels_texture_buffer_radii_;
uniform usamplerBuffer surfels_uniform_grid;


//static for testing
//vec3 _lightPos = vec3(0.0, 0.5, -4.0);



float _diffuseMaterialConstant = 0.6;
float _specularMaterialConstant = 0.2;
float _ambientMaterialConstant = 0.2;

float _specularExponent = 16;

float _ambientLightIntensity = 0.8;
float _lightIntensity = 10.0;

vec3 _object_color = vec3(1.0);

float bias = 0.01;

struct Gaussian {
    vec3 mean;
    vec3 normal;
    vec3 color;
    float radius;
};

//uniform vec3 gaussian_pos[GAUSSIAN_SAMPLES];
//uniform vec3 gaussian_normal[GAUSSIAN_SAMPLES];
//uniform vec3 gaussian_color[GAUSSIAN_SAMPLES];
//uniform float gaussian_radius[GAUSSIAN_SAMPLES];

float random (vec2 st, float seed) {
    return fract(sin(dot(st.xy,
    vec2(12.9898, 78.233)))*
    seed);
}


vec2 random_vector(vec2 st, float scale) {
    float random_angle = PI * 2 * random(st, 4378.5453123f);

    return vec2(cos(random_angle), sin(random_angle))
    * random(st, 5458.24) * scale;
}



vec3 _reflection_vector(vec3 lightDirection) {
    return 2.0 * dot(lightDirection, normalWS) * normalWS - lightDirection;
}

vec3 projectPointOnPlane(vec3 P, vec3 P0, vec3 N) {
    N = normalize(N);// Ensure normal is unit length

    float d = dot(P - P0, N);
    return P - d * N;// Projected point
}

float rayPlaneIntersection(vec3 rayOrigin, vec3 rayDir, vec3 planeNormal, vec3 planePoint) {
    float denom = dot(planeNormal, rayDir);
    if (abs(denom) > 1e-6) { // Avoid division by zero
        float t = dot(planeNormal, planePoint - rayOrigin) / denom;
        return (t >= 0.0) ? t : -1.0;// Only return valid intersections
    }
    return -1.0;// No intersection or parallel
}


float intersection_area_two_circles(vec3 center_1, float radius_1, vec3 center_2, float radius_2) {
    float d = distance(center_1, center_2);
    float d_1 = (radius_1 * radius_1 - radius_2*radius_2 + d*d)/(2.0*d);
    float d_2 = d - d_1;

    float a = radius_1*radius_1 * acos(d_1/radius_1) - d_1 * sqrt(radius_1 * radius_1 - d_1*d_1);
    float b = radius_2*radius_2 * acos(d_2/radius_2) - d_2 * sqrt(radius_2 * radius_2 - d_2*d_2);
    return a + b;
}


float distance_to_gaussian(vec3 gaussian_pos, float gaussian_radius, vec3 pos){
    return clamp(1 - distance(gaussian_pos, pos) / gaussian_radius, 0, 1);
}

float ray_circle_distance(float circle_radius, vec3 pos_circle, vec3 circle_normal, vec3 ray_origin, vec3 ray_direction){
    if (dot(circle_normal, ray_direction) > 0){
        return 0;
    }
    ray_direction = normalize(ray_direction);
    float t = rayPlaneIntersection(ray_origin, ray_direction, circle_normal, pos_circle);
    return clamp(1 - distance(pos_circle, ray_origin + t * ray_direction) / circle_radius, 0, 1);
}

float areaOfCircleOnUnitSphere(float circle_radius, vec3 pos_circle, vec3 circle_normal, vec3 sphereCenter) {
    circle_normal = normalize(circle_normal);

    vec3 point_projected = projectPointOnPlane(sphereCenter, pos_circle, circle_normal);
    vec3 center_projected = point_projected - pos_circle;

    vec3 closest_point_on_surface = pos_circle + normalize(center_projected) * min(circle_radius, length(center_projected));

    vec3 pos_relative = closest_point_on_surface - sphereCenter;


    float distance_to_rim = length(pos_relative);
    float distance_to_center = distance(sphereCenter, pos_circle);

    float cos_alpha = (dot(circle_normal, pos_relative)) / distance_to_rim;

    float angular_radius = atan(circle_radius / distance_to_rim);

    return 2.0 * PI * (1-cos(angular_radius)) * cos_alpha;
}
float areaOfTriangleOnUnitSphere(vec3 p1, vec3 p2, vec3 p3, vec3 sphereCenter) {
    vec3 a = normalize(p1 - sphereCenter);
    vec3 b = normalize(p2 - sphereCenter);
    vec3 c = normalize(p3 - sphereCenter);

    vec3 crossAB = cross(a, b);
    vec3 crossCA = cross(c, a);
    vec3 crossBC = cross(b, c);

    float alpha = acos(dot(crossBC, a)/length(crossBC));
    float beta = acos(dot(crossCA, b)/length(crossCA));
    float gamma = acos(dot(crossAB, c)/length(crossAB));

    return alpha + beta + gamma - PI;

}


float SURFELS_BUCKET_SIZE = 1.0f; //in ws units
uint SURFELS_GRID_SIZE = 128; //actual size is SURFELS_BUCKET_SIZE * SURFELS_GRID_SIZE
uint SURFEL_BUFFER_AMOUNT = 8192;


uint get_pos_in_uniform_grid() {
    vec3 ws_pos = vertexPosWs;
    ws_pos /= SURFELS_BUCKET_SIZE;
    ws_pos += SURFELS_GRID_SIZE*0.5f;
    ws_pos = floor(ws_pos);
    return uint(ws_pos.x + SURFELS_GRID_SIZE * ws_pos.y + SURFELS_GRID_SIZE * (SURFELS_GRID_SIZE * ws_pos.z));
}

void main() {


    vec3 viewDir = normalize(cameraPosWs-vertexPosWs);
    vec3 lightDir = vec3(1, 0, 0);

    //light intensity
    //float intensity = _lightIntensity/pow(distance(vertexPosWs,_lightPos),2);


    //diffuse
    float _light_diffuse_intensity = _diffuseMaterialConstant * max(dot(normalWS, lightDir), 0.0);

    //specular
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normalWS), 0.0);

    float diffEase = 1 - pow(1 - _light_diffuse_intensity, 3);
    float specIntensity  = pow(specAngle, _specularExponent)*diffEase;

    vec4 c = vec4(vec3(
    _light_diffuse_intensity  * _object_color
    + specIntensity
    + _ambientLightIntensity * _ambientMaterialConstant), 1.0);

    vec3 view_reflect = _reflection_vector(viewDir);

    //float t = areaOfTriangleOnUnitSphere(vec3(0,-3,0),vec3(4,-3,0),vec3(0,0,0),vertexPosWs);
    int p = int(get_pos_in_uniform_grid());
    uvec2 bucket_info = texelFetch(surfels_uniform_grid,p).rg;
    int bucket_size = int(bucket_info.g);
    int bucket_offset = int(bucket_info.r);
    
    vec3 d = vec3(0,0,0);
    
    for (int i = 0; i < bucket_size; i++) {
        vec3 surfle_pos = texelFetch(surfels_texture_buffer_positions_, bucket_offset + i).rgb;
        if (distance(surfle_pos, vertexPosWs) <= 0.05){
            d = texelFetch(surfels_texture_buffer_color_, bucket_offset + i).rgb;
            d.r = 1.0;

        }

    }
    
    
    FragColor = vec4(vec3(d), 1.0);
    //FragColor = vec4(vec3(abs(gaussians[3].normal)), 1.0);

}