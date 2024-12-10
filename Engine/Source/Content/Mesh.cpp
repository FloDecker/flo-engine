#include "Mesh.h"

void Mesh::initializeVertexArrays() const
{
	for (auto element : vertexArrays)
	{
		element->load();
	}
}


glm::vec3 Mesh::get_center_of_mass()
{
	if (!this->mesh_calculation_ran_)
	{
		run_mesh_calculations();
	}
	return center_of_mass_;
}

glm::mat3 Mesh::get_inertia_tensor()
{
	if (!this->mesh_calculation_ran_)
	{
		run_mesh_calculations();
	}
	return inertia_tensor_;
}


void Mesh::run_mesh_calculations()
{

	center_of_mass_ = glm::vec3(0, 0, 0);
	int vertices = 0;
	for (vertex_array* vertex_array : vertexArrays)
	{
		vertices+=vertex_array->get_vertex_array()->vertices->size();
		auto v = vertex_array->get_vertex_array()->vertices;
		for (int i = 0; i < v->size(); ++i)
		{
			center_of_mass_+=v->at(i).position;
		}
	}
	
	center_of_mass_ = center_of_mass_ / static_cast<float>(vertices);

	inertia_tensor_ = glm::mat3(0);
	for (vertex_array* vertex_array : vertexArrays)
	{
		
		auto v = vertex_array->get_vertex_array()->vertices;
		for (int i = 0; i < v->size(); ++i)
		{
			auto vec_to_point = v->at(i).position-center_of_mass_;
			inertia_tensor_+=glm::outerProduct(vec_to_point, vec_to_point);
		}
	}
	mesh_calculation_ran_ = true;
}
