#pragma once

struct ID3D12Resource;

class Texture
{
private:
	friend class TextureLoader;
	int m_Width;
	int m_Height;
	bool m_IsLoaded;
	ID3D12Resource* m_Resource;

	void Destroy();

public:
	Texture();
	~Texture();
	const bool& IsLoaded() const;
};

