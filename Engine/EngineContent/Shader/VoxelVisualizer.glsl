[vertex]
out vec3 pos_ws;
out vec3 normal_ws;
out vec3 cam_pos_ws;
void main() {
    cam_pos_ws = cameraPosWS;
    pos_ws = (mMatrix * vec4(aPos, 1.0)).xyz;
    normal_ws = (mMatrix * vec4(aNormal, 0.0)).xyz;
    vec4 vertexCamSpace =vMatrix * mMatrix * vec4(aPos, 1.0);
    gl_Position = pMatrix * vertexCamSpace;
}

[fragment]
in vec3 pos_ws;
in vec3 normal_ws;
in vec3 cam_pos_ws;

uniform sampler3D voxelData;

uniform vec3 voxel_field_lower_left;
uniform vec3 voxel_field_upper_right;
uniform float voxel_field_step_size;
uniform int voxel_field_depth;
uniform int voxel_field_height;
uniform int voxel_field_width;
#define max_iteration 1000
#define shadow_color vec3(0.1,0.1,0.2)
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


vec3 visualization_xray() {

    vec3 cam_direction = normalize(pos_ws - cam_pos_ws);
    vec3 voxel_traversal_pos = cam_pos_ws;


    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    vec3 col_out = vec3(0.0);

    for (int i = 0; i < 100; i++){
        if(!is_in_volume(voxel_traversal_pos)){
            voxel_traversal_pos+= cam_direction*0.1;
            
        } else {
        float a = world_space_coord_voxel_field_lookup(voxel_traversal_pos,box_distances).a ;
        if(a < 0.01){
            col_out+=vec3(1.0)*0.01;
        }
        voxel_traversal_pos+= cam_direction*0.1;
        }

    }
    return col_out;
}


vec3 visualization_solid() {

    vec3 cam_direction = normalize(pos_ws - cam_pos_ws);
    vec3 voxel_traversal_pos = pos_ws;


    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    vec3 col_out = vec3(0.0);


    for (int i = 0; i < 100; i++){
        if(world_space_coord_voxel_field_lookup(voxel_traversal_pos,box_distances).a > 0.0){
            if(!is_in_volume(voxel_traversal_pos)){break;}
            col_out.r+=0.05;
        }
        voxel_traversal_pos+= cam_direction*0.1;
            
    }
    return vec3(float(col_out.r > 0.01));
}

vec3 visualization_solid_df(float range_lower, float range_heigher) {

    vec3 cam_direction = normalize(pos_ws - cam_pos_ws);
    vec3 voxel_traversal_pos = cam_pos_ws;


    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    vec3 col_out = vec3(0.0);


    for (int i = 0; i < 100; i++){
        if(!is_in_volume(voxel_traversal_pos)){
            voxel_traversal_pos+= cam_direction*0.5;
            
        } else {
            float a = world_space_coord_voxel_field_lookup(voxel_traversal_pos,box_distances).a;
        if(a > range_lower &&
            a < range_heigher){
            return vec3(0.0,0.0,i/100.0);
        }
        
        voxel_traversal_pos+= cam_direction*0.1;
        }

    }
    return vec3(0.0);
}


float intersection_df(vec3 trace_start, vec3 trace_direction) {
    float max_travel_distance = 8.0*(2.0/14.0);
    trace_direction = normalize(trace_direction);
    
    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    
    vec3 in_field_pos = trace_start;
    for (int i=0; i < max_iteration; i++) {
        if (is_in_volume(in_field_pos)) {
            break;
        }
        in_field_pos+=trace_direction * 0.5;
    }

    for (int i=0; i < 400; i++) {
        float a = world_space_coord_voxel_field_lookup(in_field_pos,box_distances).a;
        float d = a * max_travel_distance;
        if (a < 0.07) {
            return 0.01*i;
        }

        if (!is_in_volume(in_field_pos)) {
            return -1;
        }
        in_field_pos+=trace_direction*d;
    }
    
    return -1.0;
}


float intersectRayPlane(vec3 rayOrigin, vec3 rayDir, vec3 planePoint, vec3 planeNormal) {
    // Calculate the dot product between the ray direction and the plane normal
    float denom = dot(planeNormal, rayDir);

    // If denom is close to 0, the ray is parallel to the plane, hence no intersection
    if (abs(denom) < 1e-6) {
        return -1.0; // No intersection
    }

    // Calculate the vector from the ray origin to the plane point
    vec3 diff = planePoint - rayOrigin;

    // Calculate the intersection distance along the ray direction
    float t = dot(diff, planeNormal) / denom;

    // If t is negative, the intersection is behind the ray origin
    if (t < 0.0) {
        return -1.0; // No intersection
    }

    return t;
}

vec3 visualization_slide(vec3 slidePos, vec3 slideNormal) {

    vec3 cam_direction = normalize(pos_ws - cam_pos_ws);
    vec3 voxel_traversal_pos = pos_ws;


    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 box_distances = abs(v_lower_right_upper_left);
    vec3 col_out = vec3(0.0);
    
    vec3 v = cam_pos_ws+cam_direction*intersectRayPlane(cam_pos_ws,cam_direction,slidePos,slideNormal);
    return vec3(world_space_coord_voxel_field_lookup(v,abs(v_lower_right_upper_left)).a);
//return v;
}


void main() {
    vec3 v_lower_right_upper_left = voxel_field_upper_right - voxel_field_lower_left;
    vec3 cam_direction = normalize(pos_ws - cam_pos_ws);

    vec3 box_distances = abs(v_lower_right_upper_left);
    float d = world_space_coord_voxel_field_lookup(pos_ws,box_distances).a;
    //if (!is_in_volume(pos_ws)) {
    //    FragColor = vec4(1.0,0.,0.,1.0);
    //    return;
    //}
    //FragColor = vec4(vec3(float(d<0.05)),1.0);    
    //FragColor = vec4(vec3(d),1.0);    
    //FragColor = vec4(visualization_solid_df(0.0,0.05)+vec3(0.1),1.0);
    FragColor = vec4(vec3(intersection_df(cam_pos_ws,cam_direction)),1.0);
    //FragColor = vec4(visualization_slide(vec3(0.,0.0,0.0),normalize(vec3(1.0,0.0,0.0))),1.0);
    //FragColor = vec4(visualization_solid_df(),1.0);
}