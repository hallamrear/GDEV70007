#include <pch.h>
#include <Rendering/IMGUIRenderable.h>

std::vector<IIMGUIRenderable*> IIMGUIRenderable::m_Instances = std::vector<IIMGUIRenderable*>();

IIMGUIRenderable::IIMGUIRenderable()
{
	m_Instances.push_back(this);
}

IIMGUIRenderable::~IIMGUIRenderable()
{
	m_Instances.erase(std::find(m_Instances.begin(), m_Instances.end(), this));
}

void IIMGUIRenderable::RenderAllIMGUIInstances()
{
	size_t instanceCount = m_Instances.size();
	for (size_t i = 0; i < instanceCount; i++)
	{
		m_Instances[i]->OnIMGUIRender();
	}
}
