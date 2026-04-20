#include "pch.h"
#include "Model.h"
#include <Rendering/Geometry/Mesh.h>

Model::Model()
{
	m_DisplayName = "Model not loaded.";
	m_Meshes = std::vector<Mesh*>();
}

Model::~Model()
{
	for (size_t i = 0; i < m_Meshes.size(); i++)
	{
		if (m_Meshes[i] != nullptr)
		{
			delete m_Meshes[i];
			m_Meshes[i] = nullptr;
		}
	}
}

const bool Model::IsLoaded() const
{
	return (m_Meshes.size() != 0);
}

const std::vector<Mesh*>& Model::GetMeshes() const
{
	return m_Meshes;
}

const std::string& Model::GetDisplayName() const
{
	return m_DisplayName;
}