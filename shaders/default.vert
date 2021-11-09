#version 450

struct Vertex
{
  float px, py, pz;
  float nx, ny, nz;
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

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;

out gl_PerVertex
{
  vec4 gl_Position;
};

void main()
{
  const Vertex vertex = vertices[gl_VertexIndex];
  const vec3 position = vec3(vertex.px, vertex.py, vertex.pz);
  const vec3 normal   = vec3(vertex.nx, vertex.ny, vertex.nz);
  gl_Position  = frameData.mvpMat * vec4(position, 1);
  out_position = vec3(frameData.modelMat * vec4(position, 1));
  out_normal   = normal;
}
