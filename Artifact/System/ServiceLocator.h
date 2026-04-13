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

public:
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
