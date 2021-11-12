#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_tangent;
layout(location = 3) in vec2 in_texcoord;

layout(location = 0) out vec4 out_color;

layout(std140, set=0, binding=0) uniform FrameDataUniform
{
  mat4 mvpMat;
  mat4 modelMat;
} frameData;

// TODO: Avoid hard-coded max textures
layout(set=0, binding=2) uniform sampler2D sBaseColorTextures[24];
layout(set=0, binding=3) uniform sampler2D sNormalTextures[24];

layout(push_constant) uniform PushConstantUniform
{
  uint textureId;
} pushConstant;

vec3 reinhard_tone_mapping(vec3 color)
{
  return color / (color + 1);
}

// Reference: https://64.github.io/tonemapping/
vec3 approx_aces_tone_mapping(vec3 color)
{
  color *= 0.6;
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0, 1);
}

void main()
{
  const vec3 N = normalize(in_normal);
  const vec3 T = normalize(in_tangent.xyz);
  const vec3 B = cross(in_normal, in_tangent.xyz) * in_tangent.w;
  const mat3 tbnMat = mat3(T, B, N);

  const vec4 base_color = texture(sBaseColorTextures[pushConstant.textureId], in_texcoord);

  const vec3 _normal = texture(sNormalTextures[pushConstant.textureId], in_texcoord).xyz * 2 - 1;
  const vec3 normal  = normalize(tbnMat * _normal);

  if (base_color.a == 0) discard; // TODO: This is not performant

  // Some arbitrary light
  const vec3  light_position  = vec3(0, 3, 0);
  const float light_intensity = 20;

  const vec3 light_dir = normalize(light_position - in_position);
  const float half_lambert_factor = max(dot(normal, light_dir), 0) * 0.5 + 0.5; // Half-lambert diffuse shading
  const float diffuse_factor = half_lambert_factor * half_lambert_factor;

  const float dist = distance(light_position, in_position);
  const float attenuation = light_intensity / (dist * dist);

  const vec3 ldr_color = attenuation * diffuse_factor * base_color.rgb;
  const vec3 hdr_color = approx_aces_tone_mapping(ldr_color);
  //const vec3 hdr_color = reinhard_tone_mapping(ldr_color);
  out_color = vec4(hdr_color, 1);
}
