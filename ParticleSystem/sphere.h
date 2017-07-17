#pragma once
#include <vector>
#include <cassert>
#include <cstddef>
#include <cmath>

static const float pi = 3.1415926535897932384626433832795029;
static const float half_pi = 3.1415926535897932384626433832795029 / 2;
static const float two_pi = 2 * 3.1415926535897932384626433832795029;

void geom_sphere(
	std::vector<float>& positions,
	std::vector<float>& normals,
	std::vector<float>& texcoords,
	std::vector<unsigned int>& indices,
	int slices, int stacks)
{
	assert(slices >= 4);
	assert(stacks >= 2);

	positions.clear();
	normals.clear();
	texcoords.clear();
	indices.clear();

	for (int i = 0; i <= stacks; i++) {
		float ty = static_cast<float>(i) / stacks;
		float lat = ty * pi;
		for (int j = 0; j <= slices; j++) {
			float tx = static_cast<float>(j) / slices;
			float lon = tx * two_pi - half_pi;
			float sinlat = sin(lat);
			float coslat = cos(lat);
			float sinlon = sin(lon);
			float coslon = cos(lon);
			float x = sinlat * coslon;
			float y = coslat;
			float z = sinlat * sinlon;
			positions.push_back(x);
			positions.push_back(y);
			positions.push_back(z);
			normals.push_back(x);
			normals.push_back(y);
			normals.push_back(z);
			texcoords.push_back(1.0f - tx);
			texcoords.push_back(1.0f - ty);
			if (i < stacks && j < slices) {
				indices.push_back((i + 0) * (slices + 1) + (j + 0));
				indices.push_back((i + 0) * (slices + 1) + (j + 1));
				indices.push_back((i + 1) * (slices + 1) + (j + 0));
				indices.push_back((i + 0) * (slices + 1) + (j + 1));
				indices.push_back((i + 1) * (slices + 1) + (j + 1));
				indices.push_back((i + 1) * (slices + 1) + (j + 0));
			}
		}
	}
}