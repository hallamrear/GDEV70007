#pragma once
#include <map>

class Renderer;

class Service
{

};

class ServiceLocator
{
private:
	//static std::map<Service, Service*> m_ServiceMap;

	static Renderer* m_RendererService;

public:
	static void Provide(Renderer* renderer);

	template<class T>
	static T* Locate();

	/*template<class T*>
	static bool Provide(T* service);

	template<class T>
	static T* Locate<>();*/
};
//
//template<>
//inline bool ServiceLocator::Provide<T>(T* service)
//{
//	Service* castedService = dynamic_cast<Service*>(service)
//
//	if (castedService != nullptr)
//	{
//		m_ServiceMap.insert({ T, service });
//	}
//}

template<>
inline Renderer* ServiceLocator::Locate<>()
{
	return m_RendererService;
}
