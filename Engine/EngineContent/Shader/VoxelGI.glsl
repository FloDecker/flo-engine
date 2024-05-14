[vertex]
out vec3 pos_ws;
void main() {
    pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
in vec3 pos_ws;
uniform sampler3D voxelData;

uniform vec3 voxel_field_lower_left;
uniform vec3 voxel_field_upper_right;
uniform float voxel_field_step_size;
uniform int voxel_field_depth;
uniform int voxel_field_height;
uniform int voxel_field_width;
#define max_iteration 1000

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
    return texture(voxelData,t);
}


//vec3 get_hit_in_field_xy(vec3 ray_start_ws, vec3 ray_direction, vec3 box_distances) {
//    float x = ray_start_ws.x;
//    
//}

void main() {
    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    vec3 direction =  normalize(vec3(1, 1, 1));
    float step_x =  voxel_field_step_size/direction.x;

    vec3 voxel_traversal_pos = vec3(0);
    if (!is_in_volume(pos_ws)) { // start pos is not inside of the voxel field
        FragColor = vec4(vec3(0.0), 1.0);
        return;
    }
    FragColor = vec4(vec3(0.0), 1.0);
    voxel_traversal_pos = pos_ws;
    
    //FragColor = vec4(world_space_coord_voxel_field_lookup(pos_ws,box_distances), 1.0);
    vec3 testCol = vec3(0.0);
    for(int i = 0; i<50;i++) {
        if (!is_in_volume(pos_ws)) { // start pos is not inside of the voxel field
            FragColor = vec4(testCol, 1.0);
            return;
        }
        FragColor = FragColor + world_space_coord_voxel_field_lookup(voxel_traversal_pos,box_distances).a*0.1;
        voxel_traversal_pos += direction*step_x;
    }

    //} else { // start pos is outside of voxel field
    //    vec3 hit_plane_x_lower_left = hit_on_plane(vec3(1, 0, 0), voxel_field_lower_left, pos_ws, direction);
    //    vec3 hit_plane_y_lower_left = hit_on_plane(vec3(0, 1, 0), voxel_field_lower_left, pos_ws, direction);
    //    vec3 hit_plane_z_lower_left = hit_on_plane(vec3(0, 0, 1), voxel_field_lower_left, pos_ws, direction);
    //    vec3 hit_plane_x_upper_right = hit_on_plane(vec3(1, 0, 0), voxel_field_upper_right, pos_ws, direction);
    //    vec3 hit_plane_y_upper_right = hit_on_plane(vec3(0, 1, 0), voxel_field_upper_right, pos_ws, direction);
    //    vec3 hit_plane_z_upper_right = hit_on_plane(vec3(0, 0, 1), voxel_field_upper_right, pos_ws, direction);
//
    //    bool hit_is_in_plane_x_lower_left = is_in_surface(voxel_field_upper_right.yz, voxel_field_lower_left.yz, hit_plane_x_lower_left.yz);
    //    bool hit_is_in_plane_y_lower_left = is_in_surface(voxel_field_upper_right.xz, voxel_field_lower_left.xz, hit_plane_y_lower_left.xz);
    //    bool hit_is_in_plane_z_lower_left = is_in_surface(voxel_field_upper_right.xy, voxel_field_lower_left.xy, hit_plane_z_lower_left.xy);
    //    bool hit_is_in_plane_x_upper_right = is_in_surface(voxel_field_upper_right.yz, voxel_field_lower_left.yz, hit_plane_x_upper_right.yz);
    //    bool hit_is_in_plane_y_upper_right = is_in_surface(voxel_field_upper_right.xz, voxel_field_lower_left.xz, hit_plane_y_upper_right.xz);
    //    bool hit_is_in_plane_z_upper_right = is_in_surface(voxel_field_upper_right.xy, voxel_field_lower_left.yz, hit_plane_z_upper_right.xy);
    //    
    //    if (hit_is_in_plane_x_lower_left || hit_is_in_plane_y_lower_left || hit_is_in_plane_z_lower_left){
//
//
    //        
//
//
    //        hits = 1;
    //    }
    //}


    //float t = distance(pos_ws, voxel_field_lower_left)* distance(pos_ws, voxel_field_upper_right);

    //c = c + texture(voxelData,pos_sized+float(i)*0.0002).rgb*0.001;

}