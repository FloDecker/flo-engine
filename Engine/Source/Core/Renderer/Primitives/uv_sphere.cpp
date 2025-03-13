#include "uv_sphere.h"

#include <numbers>

uv_sphere::uv_sphere(int n_slices, int n_stacks)
{
	auto *vertices = new std::vector<vertex>();
	auto *indices = new std::vector<unsigned int>();
	
	// add top vertex
	vertices->push_back(vertex {glm::vec3(0, 1, 0),glm::vec3(0, 1, 0), glm::vec2(0,0)});

	// generate vertices per stack / slice
	for (int i = 0; i < n_stacks - 1; i++)
	{
		constexpr auto m_pi = std::numbers::pi;
		auto phi = m_pi * double(i + 1) / double(n_stacks);
		for (int j = 0; j < n_slices; j++)
		{
			auto theta = 2.0 * m_pi * double(j) / double(n_slices);
			auto x = std::sin(phi) * std::cos(theta);
			auto y = std::cos(phi);
			auto z = std::sin(phi) * std::sin(theta);
			vertices->push_back(vertex {glm::vec3(x, y, z),glm::vec3(x, y, z), glm::vec2(0,0)});
		}
	}

	// add bottom vertex
	vertices->push_back(vertex {glm::vec3(0, -1, 0),glm::vec3(0, -1, 0), glm::vec2(0,0)});

	// add top / bottom triangles
	for (int i = 0; i < n_slices; ++i)
	{
		auto i0 = i + 1;
		auto i1 = (i + 1) % n_slices + 1;
		indices->push_back(0);
		indices->push_back(i1);
		indices->push_back(i0);
		i0 = i + n_slices * (n_stacks - 2) + 1;
		i1 = (i + 1) % n_slices + n_slices * (n_stacks - 2) + 1;
		indices->push_back(vertices->size() - 1);
		indices->push_back(i0);
		indices->push_back(i1);
	}

	// add quads per stack / slice
	for (int j = 0; j < n_stacks - 2; j++)
	{
		auto j0 = j * n_slices + 1;
		auto j1 = (j + 1) * n_slices + 1;
		for (int i = 0; i < n_slices; i++)
		{
			auto i0 = j0 + i;
			auto i1 = j0 + (i + 1) % n_slices;
			auto i2 = j1 + (i + 1) % n_slices;
			auto i3 = j1 + i;
			//mesh.add_quad(Vertex(i0), Vertex(i1),
			//			  Vertex(i2), Vertex(i3));
			indices->push_back(i0);
			indices->push_back(i1);
			indices->push_back(i2);
			
			indices->push_back(i2);
			indices->push_back(i3);
			indices->push_back(i0);
		}
	}

	
	sphere_vertex_array_ = new vertex_array(vertices , indices);
	uv_sphere::load();
}

int uv_sphere::load()
{
	return sphere_vertex_array_->load();
}

int uv_sphere::draw()
{
	return sphere_vertex_array_->draw();
}

primitive_type uv_sphere::get_primitive_type()
{
	return sphere;
}

