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

//static for testing
//vec3 _lightPos = vec3(0.0, 0.5, -4.0);

uint bitmask_surfel_amount = 0x00FFFFFF;
float total_extension = 512.0;

float _diffuseMaterialConstant = 0.6;
float _specularMaterialConstant = 0.2;
float _ambientMaterialConstant = 0.2;

float _specularExponent = 16;

float _ambientLightIntensity = 0.8;
float _lightIntensity = 10.0;

vec3 _object_color = vec3(1.0);

float bias = 0.01;

struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 color;
};

struct OctreeElement
{
    uint surfels_at_layer_amount;
    uint surfels_at_layer_pointer;
    uint next_layer_surfels_pointer[8];
};

layout(std430, binding = 0) buffer SurfelBuffer {
    Surfel surfels[];
};

layout(std430, binding = 1) buffer OctreeBuffer {
    OctreeElement octreeElements[];
};

float random (vec2 st, float seed) {
    return fract(sin(dot(st.xy,
    vec2(12.9898, 78.233)))*
    seed);
}

vec3 float_to_heat_map (float f) {
    //return mix(
    //    vec3(235.0 / 255.0 ,64.0 / 255.0 , 52.0/ 255.0) ,
    //    vec3(41.0/ 255.0, 214.0/ 255.0, 78.0/ 255.0) ,
    //   f
    //);
    return mix(
    vec3(1, 0, 0),
    vec3(0, 1, 0),
    f
    );
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


float SURFELS_BUCKET_SIZE = 1.0f;//in ws units
uint SURFELS_GRID_SIZE = 128;//actual size is SURFELS_BUCKET_SIZE * SURFELS_GRID_SIZE
uint SURFEL_BUFFER_AMOUNT = 8192;


uint get_pos_in_uniform_grid() {
    vec3 ws_pos = vertexPosWs;
    ws_pos /= SURFELS_BUCKET_SIZE;
    ws_pos += SURFELS_GRID_SIZE*0.5f;
    ws_pos = floor(ws_pos);
    return uint(ws_pos.x + SURFELS_GRID_SIZE * ws_pos.y + SURFELS_GRID_SIZE * (SURFELS_GRID_SIZE * ws_pos.z));
}

uint get_surfel_amount(uint i) {
    return i & bitmask_surfel_amount;
}

bool is_child_octree_bit_set_at(uint i, uint pos)
{
    return (i & (1u << 31-pos)) != 0;
}

uint get_pos_of_next_surfel_index_(vec3 pos_relative)
{
    uint r = 0u;
    if (pos_relative.x >= 0)
    {
        r |= (1u << 2);
    }

    if (pos_relative.y >= 0)
    {
        r |= (1u << 1);
    }

    if (pos_relative.z >= 0)
    {
        r |= (1u << 0);
    }
    return r;
}

vec3 get_next_center(vec3 current_center, vec3 pos_relative, int current_layer) {
    vec3 next_center = current_center + vec3(
    pos_relative.x >= 0 ? 1.0 : -1.0,
    pos_relative.y >= 0 ? 1.0 : -1.0,
    pos_relative.z >= 0 ? 1.0 : -1.0
    ) * 0.5f * (total_extension / pow(2.0f, current_layer + 1));
    return next_center;
}

vec3 debug_bits(uint i) {
    uint t = uint(mod(int(ceil(vertexPosWs.x)), 32));
    bool s = (i & (1u << t)) != 0;
    return vec3(s, t/32.0, 0.0);
}

vec3 get_color_from_octree(vec3 pos) {
    vec3 final_color = vec3(0.0);
    uint index = 0;

    int amount_texture_fetches = 0;//amount of texture fetches
    int amount_contribution = 0;//count the amount of correct hits 

    vec3 current_center = vec3(0, 0, 0);
    float feched_samples = 0;
    for (int current_layer = 0; current_layer < 100; current_layer++) {
        OctreeElement o = octreeElements[index];
        uint bucket_info = o.surfels_at_layer_amount;
        amount_texture_fetches++;
        //bucket contains surfels 
        uint surfels_amount = get_surfel_amount(bucket_info);
        //TODO: sample surfels

        //sample surfles from bucket:
        uint surfle_data_pointer;
        if (surfels_amount > 0) {
            surfle_data_pointer = o.surfels_at_layer_pointer;
            amount_texture_fetches++;
            //sample all of the surfels in bucket
 
            for (int i = 0; i < surfels_amount; i++) {
                Surfel s = surfels[surfle_data_pointer + i];
                amount_texture_fetches++;

                float d = distance(vertexPosWs, s.mean_r.xyz);
                if (d < s.mean_r.w) {
                    if (dot(s.normal.xyz, normalWS) > 0.1) {
                        float attenuation = 1.0f - d / s.mean_r.w;
                        final_color+=s.color.rgb*attenuation;
                        amount_contribution++;
                        feched_samples+=attenuation;
                    }
                }
            }
        }
        vec3 pos_relative = pos - current_center;
        uint index_of_next_pointer = get_pos_of_next_surfel_index_(pos_relative);
        if (is_child_octree_bit_set_at(bucket_info, int(index_of_next_pointer))) {
            //there is another child octree containing information for this texel
            index = o.next_layer_surfels_pointer[index_of_next_pointer];
            amount_texture_fetches++;
            current_center = get_next_center(current_center, pos_relative, current_layer);
        } else {
            return final_color/feched_samples;
            //return float_to_heat_map(1.0 - amount_texture_fetches * 0.001);
            //return float_to_heat_map(1.0 - amount_texture_fetches/amount_contribution * 0.01);
        }
    }
    return vec3(1, 0, 0);
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


    //float t = areaOfTriangleOnUnitSphere(vec3(0,-3,0),vec3(4,-3,0),vec3(0,0,0),vertexPosWs);
    int p = int(get_pos_in_uniform_grid());
    vec3 d = get_color_from_octree(vertexPosWs);
    
    FragColor = vec4(d, 1.0);
    //FragColor = vec4(vec3(abs(gaussians[3].normal)), 1.0);

}