#include "pch.h"
#include "Model.h"

Model::Model()
{
	m_Name = "Model not loaded.";
	m_Meshes = std::vector<Mesh*>();
	m_Textures = std::vector<ID3D12Resource*>();
}

Model::~Model()
{

}