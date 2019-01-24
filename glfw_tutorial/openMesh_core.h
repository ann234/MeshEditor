#pragma once
#define _USE_MATH_DEFINES

#pragma warning(disable:4996)

#include <deque>

#include <glm\gtx\transform.hpp>
#include <OpenMesh\Core\IO\MeshIO.hh>
#include <OpenMesh\Core\Mesh\PolyMesh_ArrayKernelT.hh>
typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;

#pragma comment(lib, "OpenMeshCore.lib")

glm::vec3 caster(OpenMesh::Vec3f src);
OpenMesh::Vec3uc f_to_uc(float r, float g, float b);
OpenMesh::Vec3uc f_to_uc(glm::vec3 color);
OpenMesh::Vec3f uc_to_f(OpenMesh::Vec3uc uc);