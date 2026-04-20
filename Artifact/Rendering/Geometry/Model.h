#pragma once

struct ID3D12Resource;
class Mesh;

class Model
{
private:
	friend class AssetLoader;
	std::string m_DisplayName;
	std::string m_FileLocation;
	std::vector<Mesh*> m_Meshes;

protected:

public:
	Model();
	~Model();

	const bool IsLoaded() const;

	const std::vector<Mesh*>& GetMeshes() const;
	const std::string& GetDisplayName() const;
};

typedef std::shared_ptr<Model> ModelRef;
typedef std::weak_ptr<Model> ModelPtr;