#include "openMesh_core.h"

glm::vec3 caster(OpenMesh::Vec3f src)
{
	return glm::vec3(src[0], src[1], src[2]);
}

OpenMesh::Vec3uc f_to_uc(float r, float g, float b)
{
	return OpenMesh::color_caster<OpenMesh::Vec3uc, OpenMesh::Vec3f>().cast(OpenMesh::Vec3f(r, g, b));
}

OpenMesh::Vec3uc f_to_uc(glm::vec3 color)
{
	float r = color.x;
	float g = color.y;
	float b = color.z;
	return OpenMesh::color_caster<OpenMesh::Vec3uc, OpenMesh::Vec3f>().cast(OpenMesh::Vec3f(r, g, b));
}

OpenMesh::Vec3f uc_to_f(OpenMesh::Vec3uc uc)
{
	return OpenMesh::color_caster<OpenMesh::Vec3f, OpenMesh::Vec3uc>().cast(uc);
}