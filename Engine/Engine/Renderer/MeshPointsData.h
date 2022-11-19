#pragma once

struct MeshPointsData
{
	std::vector<Vec3f> vertices;
	std::vector<unsigned int> indices;
	Vec3f localPosition;
	Quatf localRotation;
};