#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

hitAttributeEXT vec2 hitCoordinate;

layout(location = 0) rayPayloadInEXT vec3 hitPayload;

void main()
{
  hitPayload = vec3(1.0, 1.0, 1.0);
}