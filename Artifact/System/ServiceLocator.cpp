#include "pch.h"
#include "ServiceLocator.h"

Renderer* ServiceLocator::m_RendererService = nullptr;

void ServiceLocator::Provide(Renderer* renderer)
{
    m_RendererService = renderer;
}