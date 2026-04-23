#pragma once
#include <Rendering/IMGUIIncludes.h>

class IIMGUIRenderable
{
private:
	static std::vector<IIMGUIRenderable*> m_Instances;

public:
	IIMGUIRenderable();
	virtual ~IIMGUIRenderable() = 0;
	virtual void OnIMGUIRender() = 0;

	static void RenderAllIMGUIInstances();
};

