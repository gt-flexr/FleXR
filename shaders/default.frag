#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec4 out_color;

layout(std140, set=0, binding=0) uniform FrameDataUniform
{
  mat4 mvpMat;
  mat4 modelMat;
  mat4 normalMat;
} frameData;

void main()
{
  const vec3 light_pos = vec3(1); // Some arbitrary location
  const vec3 normal = vec3(normalize(frameData.normalMat * vec4(in_normal, 0)));
  const vec3 light_dir = normalize(light_pos - in_position);
  const float diffuse_factor = max(dot(normal, light_dir), 0.0) * 0.5 + 0.5;
  const vec3 diffuse_color = vec3(1, 0.5, 0.25);
  out_color = vec4(diffuse_factor * diffuse_color, 1);
}
