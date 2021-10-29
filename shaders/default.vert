#version 450

struct Vertex
{
  float px;
  float py;
  float pz;
};

layout(std430, set=0, binding=0) readonly buffer VerticesBuffer
{
  Vertex vertices[];
};

out gl_PerVertex
{
  vec4 gl_Position;
};

const mat3 scaleMat = mat3(
  0.5, 0.0, 0.0,
  0.0, 0.5, 0.0,
  0.0, 0.0, 0.5);

const mat3 rotX10Mat = mat3(
  1.0, 0.0, 0.0,
  0.0, +0.9848077, -0.1736482,
  0.0, -0.1736482, +0.9848077);

const mat3 rotY10Mat = mat3(
  +0.9848077, 0.0, 0.1736482,
  0.0, 1.0, 0.0,
  -0.1736482, 0.0, 0.9848077);

const mat3 rotZ10Mat = mat3(
  0.9848077, -0.1736482, 0.0,
  0.1736482, +0.9848077, 0.0,
  0.0, 0.0, 1.0);

void main()
{
  const Vertex vertex = vertices[gl_VertexIndex];
  const vec3 position = vec3(vertex.px, vertex.py, vertex.pz);
  const mat3 modelMat = rotZ10Mat * rotY10Mat * rotX10Mat * scaleMat;
  gl_Position = vec4(modelMat * position, 1);
}
