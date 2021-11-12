#version 450

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_tangent;
layout(location = 3) out vec2 out_texcoord;

out gl_PerVertex
{
  vec4 gl_Position;
};

struct Vertex
{
  float px, py, pz;     // Position
  float nx, ny, nz;     // Normal
  float tx, ty, tz, tw; // Tangent
  float tu, tv;         // Texcoord
};

layout(std140, set=0, binding=0) uniform FrameDataUniform
{
  mat4 mvpMat;
  mat4 modelMat;
} frameData;

layout(std430, set=0, binding=1) readonly buffer VerticesBuffer
{
  Vertex vertices[];
};

void main()
{
  const Vertex vertex = vertices[gl_VertexIndex];
  const vec3 position = vec3(vertex.px, vertex.py, vertex.pz);
  const vec3 normal   = vec3(vertex.nx, vertex.ny, vertex.nz);
  const vec4 tangent  = vec4(vertex.tx, vertex.ty, vertex.tz, vertex.tw);
  const vec2 texcoord = vec2(vertex.tu, vertex.tv);

  gl_Position  = frameData.mvpMat * vec4(position, 1);
  out_position = vec3(frameData.modelMat * vec4(position, 1));
  out_normal   = mat3(frameData.modelMat) * normal;
  out_tangent  = tangent;
  out_texcoord = texcoord;
}
