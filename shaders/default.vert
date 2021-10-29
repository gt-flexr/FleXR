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

const mat4 MVP = mat4(
  0.5, 0.0, 0, 0,
  0.0, 0.5, 0, 0,
  0.0, 0.0, 1, 0,
  0.0, 0.0, 0, 1);

void main()
{
  const Vertex vertex = vertices[gl_VertexIndex];
  const vec3 position = vec3(vertex.px, vertex.py, vertex.pz);
  gl_Position = MVP * vec4(position, 1);
}
