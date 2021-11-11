#version 450

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_texcoord;

out gl_PerVertex
{
  vec4 gl_Position;
};

struct Vertex
{
  float px, py, pz;
  float nx, ny, nz;
  float tu, tv;
};

layout(std140, set=0, binding=0) uniform FrameDataUniform
{
  mat4 mvpMat;
  mat4 modelMat;
  mat4 normalMat;
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
  const vec2 texcoord = vec2(vertex.tu, vertex.tv);
  gl_Position  = frameData.mvpMat * vec4(position, 1);
  out_position = vec3(frameData.modelMat * vec4(position, 1));
  out_normal   = normal;
  out_texcoord = texcoord;
}
