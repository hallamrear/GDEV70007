#pragma once
#include <map>

class Renderer;
class AssetManager;
class World;

class ServiceLocator
{
private:
	static Renderer* m_RendererService;
	static AssetManager* m_AssetManager;
	static World* m_World;

public:
	static void Provide(Renderer* renderer);
	static void Provide(AssetManager* assetManager);
	static void Provide(World* world);

	template<class T>
	static T* Locate();
};

template<>
inline Renderer* ServiceLocator::Locate<>()
{
	return m_RendererService;
}

template<>
inline AssetManager* ServiceLocator::Locate<>()
{
	return m_AssetManager;
}

template<>
inline World* ServiceLocator::Locate<>()
{
	return m_World;
}
