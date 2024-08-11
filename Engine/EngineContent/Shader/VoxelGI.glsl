[vertex]
out vec3 pos_ws;
out vec3 normal_ws;
void main() {
    pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    normal_ws = (mMatrix * vec4(aNormal, 0.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
in vec3 pos_ws;
in vec3 normal_ws;
uniform sampler3D voxelData;

uniform vec3 voxel_field_lower_left;
uniform vec3 voxel_field_upper_right;
uniform float voxel_field_step_size;
uniform int voxel_field_depth;
uniform int voxel_field_height;
uniform int voxel_field_width;
#define max_iteration 100
#define step_length 0.1
#define shadow_color vec3(0.1, 0.1, 0.2)
//returns the multiplactor for v so that it gets scaled so that the resulting vector is step_size longer in x
float step_length_for_x(vec3 v, float step_size){
    return step_size/v.x;
}

float step_length_for_y(vec3 v, float step_size){
    return step_size/v.y;
}

bool is_in_surface(vec2 upper_right_edge, vec2 lower_left_edge, vec2 pos) {
    return pos.x < upper_right_edge.x && pos.x > lower_left_edge.x && pos.y < upper_right_edge.y && pos.y > lower_left_edge.y;
}

bool is_in_volume(vec3 pos) {
    return pos.x > voxel_field_lower_left.x && pos.y > voxel_field_lower_left.y && pos.z > voxel_field_lower_left.z &&
    pos.x < voxel_field_upper_right.x && pos.y < voxel_field_upper_right.y && pos.z < voxel_field_upper_right.z;
}

vec3 hit_on_plane(vec3 plane_normal, vec3 point_on_plane, vec3 ray_origin, vec3 direction){
    vec3 ray_direction_normalized = normalize(direction);
    float t = dot(point_on_plane-ray_origin, plane_normal) / dot(plane_normal, ray_direction_normalized);
    return ray_origin + t * ray_direction_normalized;
}

vec4 world_space_coord_voxel_field_lookup(vec3 pos, vec3 voxel_field_size){
    vec3 t = pos - voxel_field_lower_left;
    t = t / voxel_field_size;
    return texture(voxelData, t);
}


vec3 calculateNormalFromDistanceFunction(vec3 p, vec3 voxel_field_size) {
    //Sample the distance function at the nearby points
    float distance = 0.1;
    float dx = (world_space_coord_voxel_field_lookup(p + vec3(distance, 0.0, 0.0), voxel_field_size) -
    world_space_coord_voxel_field_lookup(p - vec3(distance, 0.0, 0.0), voxel_field_size)).a;
    float dy = (world_space_coord_voxel_field_lookup(p + vec3(0.0, distance, 0.0), voxel_field_size) -
    world_space_coord_voxel_field_lookup(p - vec3(0.0, distance, 0.0), voxel_field_size)).a;
    float dz = (world_space_coord_voxel_field_lookup(p + vec3(0.0, 0.0, distance), voxel_field_size) -
    world_space_coord_voxel_field_lookup(p - vec3(0.0, 0.0, distance), voxel_field_size)).a;

    // Construct the gradient vector
    vec3 gradient = -vec3(dx, dy, dz);

    // Normalize to get the normal
    vec3 normal = normalize(gradient);

    return normal;
}

float intersection(vec3 trace_start, vec3 trace_direction, vec3 voxel_field_size) {
    float distance_0 = 0.0;
    float distance_1 = 0.0;
    float distance_2 = 0.0;
    
    for (int i = 0; i < max_iteration; i++){
        distance_0 = distance_1;
        distance_1 = distance_2;
        
        vec3 current_pos = trace_start + trace_direction*i*step_length;
        if (!is_in_volume(current_pos)) {
            return 0.5;
        }
        distance_2 = world_space_coord_voxel_field_lookup(current_pos,voxel_field_size).a;
        if (distance_0 - distance_1 < 0.0 && distance_1 - distance_2 > 0.0) {
            return 1.0;
        }
    }
    
    return -1.0;
}

void main() {

    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    vec3 direction =  normalize(vec3(1, 1, 1));
    
    vec3 surfaceNormal = calculateNormalFromDistanceFunction(pos_ws, box_distances);
    float d = intersection(pos_ws + normal_ws*0.1, direction ,box_distances);
    
    FragColor = vec4(surfaceNormal, 1.0);

    FragColor = vec4(vec3(d), 1.0);


}