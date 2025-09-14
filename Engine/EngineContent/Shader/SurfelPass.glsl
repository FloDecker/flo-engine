[vertex]
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aUv;
}

[fragment]

layout (location = 0) out vec4 gSurfel;
layout (location = 1) out vec4 surfel_metadata_0;
layout (location = 2) out vec4 surfel_metadata_1;
in vec2 TexCoords;

uniform sampler2D gPos;
uniform sampler2D gNormal;


//surfels


#define PI 3.14159265359
#define AREA_UNIT_SPHERE (4.0*PI)
#define FLOAT_MAX 2000000.0
#define MAX_LIGHT_SAMPLES FLOAT_MAX
#define GAUSSIAN_SAMPLES 512
#define MAX_LAYERS 9

#define ICLINATION_MAX 0.9f
#define ICLINATION_MIN 0.7f
#define DISTANCE_MAX 1.0f


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
    vec4 albedo;
    vec4 radiance_ambient; //radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface; //radiance contribution from direct light and surface
    uint[8] copy_locations; //global adresses where this exact surfel can be found
};

struct OctreeElement
{
    uint surfels_at_layer_amount;
    uint surfels_at_layer_pointer;
    uint next_layer_surfels_pointer[8];
};


//front buffers
layout(std430, binding = 4) readonly buffer SurfelBuffer {
    Surfel surfels[];
};

layout(std430, binding = 6) readonly buffer OctreeBuffer {
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



vec3 _reflection_vector(vec3 lightDirection, vec3 normalWS) {
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

uint get_surfel_amount(uint i) {
    return i & bitmask_surfel_amount;
}

bool is_child_octree_bit_set_at(uint i, uint pos)
{
    return (i & (1u << 31-pos)) != 0;
}

uint get_next_octree_index_(vec3 pos_relative)
{
    return (uint(pos_relative.x >= 0.0) << 2) |
    (uint(pos_relative.y >= 0.0) << 1) |
    (uint(pos_relative.z >= 0.0) << 0);
}

vec3 get_next_center(vec3 current_center, vec3 pos_relative, int current_layer) {
    vec3 next_center = current_center + vec3(
    pos_relative.x >= 0 ? 1.0 : -1.0,
    pos_relative.y >= 0 ? 1.0 : -1.0,
    pos_relative.z >= 0 ? 1.0 : -1.0
    ) * 0.5f * (total_extension / pow(2.0f, current_layer + 1));
    return next_center;
}

vec3 debug_bits(uint i, vec3 vertexPosWs) {
    uint t = uint(mod(int(ceil(vertexPosWs.x)), 32));
    bool s = (i & (1u << t)) != 0;
    return vec3(s, t/32.0, 0.0);
}

bool is_ws_pos_contained_in_bb(vec3 pos, vec3 bb_min, vec3 extension) {
    vec3 bb_max = bb_min + extension;
    return
    pos.x <= bb_max.x && pos.x >= bb_min.x &&
    pos.y <= bb_max.y && pos.y >= bb_min.y &&
    pos.z <= bb_max.z && pos.z >= bb_min.z;

}

vec3 get_ao_color(){
    return vec3(80.0/255.0,156.0/255.0,250.0/255.0) * 0.5;
}

const float distance_reciprocal = 1.0f / DISTANCE_MAX;

vec3 get_color_from_octree(vec3 pos, vec3 normal_ws, out int amount_texture_fetches, out int amount_innceseary_fetches, out float surfel_coverage, out float min_samples, out float min_sample_level, out vec3 min_sample_center, out vec3 avg_octree_color) {
    surfel_coverage = 0;
    float surfel_total_attenuation = 0;
    min_samples = MAX_LIGHT_SAMPLES;
    min_sample_level = 0;
    if (!is_ws_pos_contained_in_bb(pos, vec3(- total_extension * 0.5f), vec3(total_extension))) {
        return vec3(0.0);
    }
    vec3 final_color = vec3(0.0);
    avg_octree_color  = vec3(0.0);
    int samples_radus_independend = 0;
    uint index = 0;
    amount_texture_fetches = 0;//amount of texture fetches

    vec3 current_center = vec3(0, 0, 0);
    for (int current_layer = 0; current_layer < MAX_LAYERS; current_layer++) {
        OctreeElement o = octreeElements[index];
        uint bucket_info = o.surfels_at_layer_amount;
        amount_texture_fetches++;
        //bucket contains surfels 
        uint surfels_amount = get_surfel_amount(bucket_info);


        //sample surfles from bucket:
        uint surfle_data_pointer;
        if (surfels_amount > 0) {
            surfle_data_pointer = o.surfels_at_layer_pointer;
            amount_texture_fetches++;
            for (int i = 0; i < surfels_amount; i++) {
                Surfel s = surfels[surfle_data_pointer + i];
                amount_texture_fetches++;


                float distance_difference = max(1.0f - distance(pos, s.mean_r.xyz) / s.mean_r.w, 0.0);
                float normal_difference = smoothstep(ICLINATION_MIN, ICLINATION_MAX, max(dot(s.normal.xyz, normal_ws),0.0));
                float distance_to_surfel = max(1.0f - abs(dot(pos-s.mean_r.xyz, s.normal.xyz)) * distance_reciprocal, 0.0);
                float surfel_sample_attenuation = smoothstep(1,32,s.radiance_ambient.w);


                float attenuation =  distance_difference * normal_difference  * distance_to_surfel;
                

                avg_octree_color+=s.radiance_ambient.rgb;
                final_color+=s.radiance_ambient.rgb*attenuation * surfel_sample_attenuation;
                surfel_total_attenuation += attenuation * surfel_sample_attenuation;

                surfel_coverage += attenuation;
                samples_radus_independend++;


                //use this to prioiritize surfels that havent been sampled a lot
                if (attenuation > 0 && s.radiance_ambient.w < min_samples) {
                    min_samples = s.radiance_ambient.w;
                    min_sample_level = float(current_layer);
                    min_sample_center = s.mean_r.xyz;
                }

            }
        }
        vec3 pos_relative = pos - current_center;
        uint index_of_next_pointer = get_next_octree_index_(pos_relative);
        if (is_child_octree_bit_set_at(bucket_info, int(index_of_next_pointer))) {
            //there is another child octree containing information for this texel
            index = o.next_layer_surfels_pointer[index_of_next_pointer];
            amount_texture_fetches++;
            current_center = get_next_center(current_center, pos_relative, current_layer);
        } else {
            avg_octree_color/=float(samples_radus_independend + 0.01);
            if (surfel_total_attenuation > 0) {
                return final_color/surfel_total_attenuation;
            } else {
                return avg_octree_color;
            }

        }
    }

    avg_octree_color/=float(samples_radus_independend + 0.01);
    return avg_octree_color;
}

void main()
{
    vec3 normal_ws = vec3(texture(gNormal, TexCoords));
    vec3 pos_ws = vec3(texture(gPos, TexCoords));

    int amount_texture_fetches;
    int amount_innceseary_fetches;
    float surfel_coverage = 0.0;
    float min_samples;//returns the minimal amount of surfle samples at this fragments location
    float min_sample_level;//returns the octree level of the minimal level surfel
    vec3 min_sample_center;
    vec3 avg_octree_color;
    vec3 d = get_color_from_octree(pos_ws, normal_ws, amount_texture_fetches, amount_innceseary_fetches, surfel_coverage, min_samples, min_sample_level, min_sample_center, avg_octree_color);


    OctreeElement f;
    vec3 x_v = surfels[0].mean_r.xyz;
    //uint x = octreeElements[7].surfels_at_layer_pointer;

    gSurfel = vec4(d, surfel_coverage);
    surfel_metadata_0 = vec4(avg_octree_color, min_samples);
    surfel_metadata_1 = vec4(min_sample_center, min_sample_level);


}