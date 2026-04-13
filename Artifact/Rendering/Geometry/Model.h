#pragma once

struct ID3D12Resource;
class Mesh;

class Model
{
private:
	friend class GeometryLoader;
	std::string m_Name;
	std::vector<Mesh*> m_Meshes;
	std::vector<ID3D12Resource*> m_Textures;

protected:

public:
	Model();
	~Model();

};

