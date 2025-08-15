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
out vec4 FragColor;

in vec2 TexCoords;


uniform sampler2D gPos;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gEmissive;
uniform sampler2D dpeth_framebuffer;
uniform sampler2D gRoughnessMetallicAo;
uniform sampler2D light_map;
uniform sampler2D gSurfels;
uniform sampler2D surfel_framebuffer_metadata_0;
uniform sampler2D surfel_framebuffer_metadata_1;
uniform usampler2D gRenderFlags;
uniform sampler2D light_pass_result;


uniform sampler2D direct_light_map_texture;
layout (std140,  binding = 1) uniform DIRECT_LIGHT_UNIFORMS
{
	vec3 direct_light_direction;
	float direct_light_intensity;
	vec3 direct_light_color;
	float direct_light_light_angle;
	mat4 direct_light_light_space_matrix;
};
#define OCTREE_TOTOAL_EXTENSION 512

uniform vec3 cameraPosWs;
//surfels

struct Surfel {
    vec4 mean_r;
    vec4 normal;
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

struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 inverse_direction;
    float max_length;
};
//#define DEBUG_SURFELS
#ifdef DEBUG_SURFELS
//front buffers
layout(std430, binding = 4) readonly buffer SurfelBuffer {
    Surfel surfels[];
};

layout(std430, binding = 6) readonly buffer OctreeBuffer {
    OctreeElement octreeElements[];
};



uint bitmask_surfel_amount = 0x00FFFFFF;

uint get_surfel_amount(uint i) {
    return i & bitmask_surfel_amount;
}

bool is_child_octree_bit_set_at(uint i, uint pos)
{
    return (i & (1u << 31-pos)) != 0;
}

bool are_all_child_octree_bits_empty(uint i)
{
    return (bitmask_surfel_amount & i) == 0;
}


//from https://iquilezles.org/articles/intersectors/
bool ray_surfel_intersection(Surfel s, Ray r, out vec3 hit_location)
{
    if (dot(s.normal.xyz, r.direction) > 0) return false;

    vec3  o = r.origin - s.mean_r.xyz;
    float t = -dot(s.normal.xyz,o)/dot(r.direction,s.normal.xyz);
    if (t < 0 ) return false;
    vec3  q = o + r.direction * t;
    hit_location = r.origin + r.direction*t;
    return (dot(q,q)<s.mean_r.w * s.mean_r.w);
}


//adapted from https://iquilezles.org/articles/intersectors/
// axis aligned box centered at the origin, with size boxSize
bool boxIntersection(in Ray r, float boxSize, vec3 boxStartWS, out float distance, out float distanceNear, out float distanceFar)
{
    
    vec3 origin = r.origin - boxStartWS - boxSize; // transform the origin 
    vec3 n = r.inverse_direction * origin;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(r.inverse_direction)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0){
        return false;
    }
    distanceNear = tN;
    distanceFar = tF;
    if (tN < 0.0) {
        distance = 0.0;
    } else {
        distance = tN;
    }
    return true;
}

void insertSorted(inout uint id_array[8], inout float distance_array[8], inout int count, uint new_id, float new_distance) {
    // If array is full and new distance is greater than the largest, skip insert
    if (count == 8 && new_distance >= distance_array[7]) {
        return;
    }

    // Find insert position
    int insert_pos = 0;
    while (insert_pos < count && distance_array[insert_pos] < new_distance) {
        insert_pos++;
    }

    // Shift values to the right
    int limit = (count < 8) ? count : 7; // If full, drop the last element
    for (int i = limit; i > insert_pos; --i) {
        distance_array[i] = distance_array[i - 1];
        id_array[i] = id_array[i - 1];
    }

    // Insert new value
    distance_array[insert_pos] = new_distance;
    id_array[insert_pos] = new_id;

    // Increase count if not full
    if (count < 8) {
        count++;
    }
}

int get_ordered_child_traversal(float extension_parent, vec3 parent_min, Ray r, out uint[8] ordered_ids){
    
    float child_size = extension_parent * 0.5f;
    int length_ids = 0;
    
    uint[8] id_array;
    float[8] distance_array;
    
    for (int i = 0; i < 8; i++) {

        vec3 offset = vec3(
        (i & (1<<2)) != 0 ? 1.0 : 0.0,
        (i & (1<<1)) != 0 ? 1.0 : 0.0,
        (i & (1<<0)) != 0 ? 1.0 : 0.0
        );

        vec3 child_min = parent_min + offset * child_size;
        float d;
        float _;
        float _1;
        if (boxIntersection(r, child_size, child_min, d, _, _1)){
            //the ray intersects the aabb
            //bubble sort distance
            insertSorted(id_array, distance_array, length_ids, i, d);
        }
        
   
    }
    ordered_ids = id_array;
    return length_ids;
}

bool traverseHERO(Ray ray, out vec3 c, out float d, out vec4 debug_data) {
    const int MAX_DEPTH = 3;
    const int MAX_STACK = 32;

    // Stack to simulate traversal
    uint node_ids_stack[MAX_STACK];
    float node_size_stack[MAX_STACK];
    vec3 node_min_stack[MAX_STACK];
    node_ids_stack[0] = 0;
    node_size_stack[0] = OCTREE_TOTOAL_EXTENSION;
    node_min_stack[0] = vec3(-OCTREE_TOTOAL_EXTENSION*0.5f);

    int stackPtr = 0;

    float closest_hit = 10000000;
    vec3 current_best_hit;
    bool has_hit = false;

    for (int tries = 0; tries < 1000; tries++) {
        OctreeElement o = octreeElements[node_ids_stack[stackPtr]];
        float current_bucket_size = node_size_stack[stackPtr];
        vec3 current_bucket_min = node_min_stack[stackPtr];
        if (stackPtr < 0) {
            debug_data.r = tries;
            return has_hit;
        }


        float near_distance;
        float far_distance;
        float _;
        boxIntersection(ray, current_bucket_size, current_bucket_min, _, near_distance, far_distance);
        if (near_distance > closest_hit) {
            debug_data.r = tries;
            return true;
        }
        
        if (near_distance > ray.max_length) {
            debug_data.r = tries;

            return has_hit;
        }


        

        //check if there are surfels hit on the current layer 
        uint surfels_amount = get_surfel_amount(o.surfels_at_layer_amount);
        if (surfels_amount > 0) {
            uint surfle_data_pointer = o.surfels_at_layer_pointer;
            for (int i = 0; i < surfels_amount; i++) {
                Surfel s = surfels[surfle_data_pointer + i];
                vec3 hit_location;
                debug_data.g++;
                if (ray_surfel_intersection(s, ray, hit_location)) {
                    d = distance(hit_location, ray.origin);
                    if (d < closest_hit) {
                        c = s.radiance_direct_and_surface.xyz;
                        closest_hit = d;
                        current_best_hit = hit_location;
                        has_hit =true;
                    }
                }
            }
        }
        stackPtr--;

        float children_size = current_bucket_size * 0.5f;

        uint[8]ordered_ids;
        
        //returns sorting from lowest distance to hightest
        int intersected_children = get_ordered_child_traversal(current_bucket_size, current_bucket_min, ray ,ordered_ids);
        //for (uint morton = 0; morton < 8 ; morton++) {
        for (int i = intersected_children - 1; i >= 0; i--) {
            uint morton = ordered_ids[i];
            
            if (!is_child_octree_bit_set_at(o.surfels_at_layer_amount, morton)) continue;
            uint childIndex = o.next_layer_surfels_pointer[morton];
            
            vec3 offset = vec3(
            (morton & (1u<<2)) != 0 ? 1.0 : 0.0,
            (morton & (1u<<1)) != 0 ? 1.0 : 0.0,
            (morton & (1u<<0)) != 0 ? 1.0 : 0.0
            );
            vec3 child_min = current_bucket_min + offset * children_size;
            if (stackPtr < MAX_STACK) {
                stackPtr++;
                node_ids_stack[stackPtr] = childIndex;
                node_size_stack[stackPtr] = children_size;
                node_min_stack[stackPtr] = child_min;
            }

        }
    }
    c = vec3(0,0,1);
    return false;
}

#endif

#define PI 3.14159265359
#define EXPOSURE 1.5

float total_extension = 512.0;

float _diffuseMaterialConstant = 0.8;
float _specularMaterialConstant = 0.5;
float _ambientMaterialConstant = 0.5;

float _specularExponent = 4;

float _ambientLightIntensity = 1;

vec3 _object_color = vec3(1.0);

float bias = 0.01;
struct Surfel {
    vec4 mean_r;
    vec4 normal;
    vec4 radiance_ambient; //radiance without surface irradiance and direct light 
    vec4 radiance_direct_and_surface; //radiance contribution from direct light and surface
    uint[8] copy_locations; //global adresses where this exact surfel can be found
};


struct AllocationMetadata{
    uint surfel_bucket_pointer;
    uint surfel_octree_pointer;
    uint debug_int_32;

};


layout(std430, binding = 2) buffer AllocationMetadataBuffer {
    AllocationMetadata allocationMetadata[];
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

vec3 debug_bits(uint i, vec3 vertexPosWs) {
    uint t = uint(mod(int(ceil(vertexPosWs.x)), 32));
    bool s = (i & (1u << t)) != 0;
    return vec3(s, t/32.0, 0.0);
}


//post processing

float kernel[9] = float[](
1.0 / 16, 2.0 / 16, 1.0 / 16,
2.0 / 16, 4.0 / 16, 2.0 / 16,
1.0 / 16, 2.0 / 16, 1.0 / 16
);

vec3 get_distance_blur(float distance) {


    float offset = 1.0 / distance;

    vec2 offsets[9] = vec2[](
    vec2(-offset, offset),
    vec2(0.0f, offset),
    vec2(offset, offset),
    vec2(-offset, 0.0f),
    vec2(0.0f, 0.0f),
    vec2(offset, 0.0f),
    vec2(-offset, -offset),
    vec2(0.0f, -offset),
    vec2(offset, -offset)
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(gAlbedo, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        col += sampleTex[i] * kernel[i];
    }
    return col;
}

vec3 gamma_correction(vec3 hdrColor) {
    const float gamma = 1.0;

    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * EXPOSURE);    // gamma correction 
    
    return pow(mapped, vec3(1.0 / gamma));

}

void main()
{

    //vec2 TexCoords_scaled = TexCoords * vec2(2.0f); 
     vec2 TexCoords_scaled = TexCoords; 
    
    vec3 albedo = vec3(texture(gAlbedo, TexCoords_scaled));
    vec3 normal_ws = vec3(texture(gNormal, TexCoords_scaled));
    vec3 pos_ws = vec3(texture(gPos, TexCoords_scaled));
    vec3 emissive = vec3(texture(gEmissive, TexCoords_scaled));
    vec3 RoughnessMetallicAo = vec3(texture(gRoughnessMetallicAo, TexCoords_scaled));
    vec3 LightPass = vec3(texture(light_pass_result, TexCoords_scaled));
    vec4 surfel_buffer = vec4(texture(gSurfels, TexCoords_scaled));
    vec4 surfel_metadata_0 = vec4(texture(surfel_framebuffer_metadata_0, TexCoords_scaled));
    vec4 surfel_metadata_1 = vec4(texture(surfel_framebuffer_metadata_1, TexCoords_scaled));
    float depth = texture(dpeth_framebuffer, TexCoords_scaled).x;
    uint flags = texture(gRenderFlags, TexCoords_scaled).r;
    vec3 lightmap = texture(direct_light_map_texture, TexCoords_scaled).rgb;

    
    int amount_texture_fetches;
    int amount_innceseary_fetches;
   // vec3 d = get_color_from_octree(pos_ws, normal_ws, amount_texture_fetches, amount_innceseary_fetches);
    vec3 heat_map_texture_fetches = float_to_heat_map(1.0 - amount_texture_fetches * 0.01);


    uint x = allocationMetadata[0].debug_int_32;
    vec3 bit_debug = debug_bits(x, TexCoords.xxx * 128.0);
    vec3 final_color = vec3(0.0);
    LightPass = gamma_correction(LightPass);
    FragColor = vec4(LightPass,  1.0);
    //FragColor = vec4(surfel_metadata_0.rgb,  1.0);
    
    #ifdef DEBUG_SURFELS
    vec3 ray_direction = normalize(pos_ws - cameraPosWs);
    vec3 c = vec3(0.0f);
    float d;
    Ray r;
    r.direction = ray_direction;
    r.origin = cameraPosWs;
    r.inverse_direction = 1.0f/ray_direction;
    r.max_length = 100;
    vec3 hit_location;
    Surfel s;
    s.normal = vec4(normalize(vec3(1,0,0)),0);
    s.mean_r = vec4(10.0,10.0,10.0,1.0);
    //c = vec3(float(ray_surfel_intersection(s, r, hit_location)));
    vec4 debug_data;
    bool b= traverseHERO(r,c, d,debug_data);
    FragColor = float(b)*vec4((c + float(b)*0.1f) * 1.0f + LightPass * 0.0f,  1.0);
    FragColor = debug_data.gggg/1000.0f;

    #endif 
    return;
    if(TexCoords.y < 0.1f) {
        FragColor = vec4(bit_debug, 1.0);
        return;
    }
    
    if (TexCoords_scaled.x < 1.0) {
        if (TexCoords_scaled.y > 1.0) {

            FragColor = vec4(clamp(normal_ws.rgb, vec3(0),vec3(1)) , 1.0);
        } else {
            FragColor = vec4(surfel_metadata_0.www*0.01,1.0);
        }
    } else {
        if (TexCoords_scaled.y > 1.0) {
            vec3 heat_map_texture_fetches = float_to_heat_map(1.0 - surfel_metadata_0.r * 0.01);

            FragColor = vec4(heat_map_texture_fetches, 1.0);

        } else {
            FragColor = vec4(final_color, 1.0);

        }
    }

}