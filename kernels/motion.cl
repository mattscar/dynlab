/*
struct SphereData {
  glm::vec3 center; float radius;   [0]
  glm::vec4 acceleration;           [1]
  glm::vec4 old_velocity;           [2]
  glm::vec4 new_velocity;           [3]
  glm::vec4 displacement;           [4]
};
*/

#define obj_center center_rad.s012
#define coll_center coll_test.s012
#define obj_radius center_rad.s3
#define coll_radius coll_test.s3

__kernel void collision_detection(__global float4* obj_global) {

  float4 center_rad, obj_velocity, coll_test, coll_velocity, rad_vector;
  float rad_sum;
  int offset;

  if(get_global_id(0) < NUM_OBJECTS) {

    // Read parameters into private memory
    offset = get_global_id(0) * VECS_PER_OBJECT;
    center_rad = obj_global[offset];
    obj_velocity = obj_global[offset + 2];

    // Test for collision with other objects
    for(int i=0; i<NUM_OBJECTS; i++) {
      if(i != get_global_id(0)) {
        coll_test = obj_global[i * VECS_PER_OBJECT];
        rad_sum = obj_radius + coll_radius;
        rad_vector = (float4)(coll_center - obj_center, 0.0f);
        
        if(length(rad_vector) <= rad_sum) {
        //  && dot(rad_vector, obj_velocity) > 0.0f

          // Read old velocity for object and collision object
          coll_velocity = obj_global[i * VECS_PER_OBJECT + 2];

          // Update velocity according to equation:
          // new_velocity = (v1*(m1-m2) + 2*m2*v2)/(m1+m2)

          obj_global[offset + 1] -= 0.015f * rad_vector;
          obj_global[offset + 1] *= 0.8f;

          obj_global[offset + 3] = 
            (obj_velocity * (obj_radius - coll_radius) + 
              2*coll_radius*coll_velocity)/rad_sum;
        }
      }
    }
  }
}

__kernel void update(__global float4* obj_global, float2 dims, float delta_t) {

  if(get_global_id(0) < NUM_OBJECTS) {

    float4 center_rad, new_velocity, displacement;

    // Find position in memory
    obj_global += get_global_id(0) * VECS_PER_OBJECT;

    // Read new velocity into private memory
    center_rad = obj_global[0];
    new_velocity = obj_global[3];

    // Update kinematic parameters
    new_velocity += obj_global[1] * delta_t;   // New velocity = acceleration * dt
    if(length(new_velocity) < 0.6f) {
      new_velocity *= -1.5f;
    }
    
    displacement = new_velocity * delta_t;     // Displacement = velocity * dt
    center_rad += displacement;                // Center += displacement

    /* Detect whether object has collided with the ground */
    if(center_rad.y <= center_rad.w && new_velocity.y < 0.0f) {
       obj_global[1].y += 0.01f;
       new_velocity.y *= -1.0f;
    }

    /* Detect whether object has collided with the left wall */
    else if(center_rad.x <= center_rad.w && new_velocity.x < 0.0f) {
       obj_global[1].x += 0.01f;
       new_velocity.x *= -1.0f;
    }

    /* Detect whether object has collided with the upper wall */
    else if(center_rad.y >= (dims.y-center_rad.w) && new_velocity.y > 0.0f) {
       obj_global[1].y -= 0.01f;
       new_velocity.y *= -1.0f;
    }

    /* Detect whether object has collided with the right wall */
    else if(center_rad.x >= (dims.x-center_rad.w) && new_velocity.x > 0.0f) {
       obj_global[1].x -= 0.01f;
       new_velocity.x *= -1.0f;
    }
    
    /* Detect whether object has collided with the positive wall */
    else if(center_rad.z >= 1.0f && new_velocity.z > 0.0f) {
       obj_global[1].z -= 0.01f;
       new_velocity.z *= -1.0f;
    }
    
    /* Detect whether object has collided with the negative wall */
    else if(center_rad.z <= -6.5f && new_velocity.z < 0.0f) {
       obj_global[1].z += 0.01f;
       new_velocity.z *= -1.0f;
    }

    obj_global[0] = center_rad;
    obj_global[2] = new_velocity;
    obj_global[3] = new_velocity;
    obj_global[4] = displacement;              // Update displacement
  }
}

__kernel void motion(__global float* vbo, __global float4* obj_data) {

  float3 vertex;

  if(get_global_id(0) < NUM_VERTICES * NUM_OBJECTS) {

    obj_data += (get_global_id(0)/NUM_VERTICES) * VECS_PER_OBJECT;

    vertex = vload3(get_global_id(0), vbo);
    vertex += obj_data[4].s012;
    vstore3(vertex, get_global_id(0), vbo);
  }
}