#include "pch.h"
#include "ServiceLocator.h"

Renderer* ServiceLocator::m_RendererService = nullptr;
AssetManager* ServiceLocator::m_AssetManager = nullptr;
World* ServiceLocator::m_World = nullptr;

void ServiceLocator::Provide(Renderer* renderer)
{
    m_RendererService = renderer;
}

void ServiceLocator::Provide(AssetManager* assetManager)
{
    m_AssetManager = assetManager;
}

void ServiceLocator::Provide(World* world)
{
    m_World = world;
}
