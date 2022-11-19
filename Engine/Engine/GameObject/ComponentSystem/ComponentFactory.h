#pragma once

#include "Engine\EngineOrder.h"
#include "Engine/CopyComponentUsingReflect.h"

/*
	SUMMARY:

	COMPONENT(type, name): Uses reflection to clone itself, this is used in most cases
	COMPONENT_MASK(type, name, mask): Uses reflection to clone itself, with a mask to determine update order
	COMPONENT_SINGLETON(type, name): Uses reflection to clone itself, a singleton component, cheap access performance, only one can exist in each scene

	COMPONENT_COPYABLE(type, name): Uses copy constructor to clone itself, makes the component need to be copyable, only used in specific scenarios when reflection does not suffice
	COMPONENT_COPYABLE(type, name, mask): Uses copy constructor to clone itself, makes the component need to becopyable, only used in specific scenarios, mask is order it updates
*/

// A component that runs normal UPDATE
#define COMPONENT(type, name)											\
private:																\
class ComponentRegisterHelper											\
{																		\
public:																	\
	ComponentRegisterHelper()											\
	{																	\
		::Engine::ComponentFactory::GetInstance().RegisterComponent<type>(name, ::Engine::UPDATE);\
	}																	\
};																		\
																		\
static inline ComponentRegisterHelper __ourComponentRegisterHelper__;	\
																		\
type* Clone() const override {											\
	auto c = DBG_NEW type();											\
																		\
	type& this_ = const_cast<type&>(*this);								\
																		\
																		\
	c->myID = this_.myID;												\
																		\
	c->myGameObject = this_.myGameObject;								\
	c->OnConstruct();													\
																		\
	CopyComponentUsingReflect(&this_, c);								\
	return c;															\
}																		\
public:																	

#define COMPONENT_MASK(type, name, mask)								\
private:																\
class ComponentRegisterHelper											\
{																		\
public:																	\
	ComponentRegisterHelper()											\
	{																	\
		::Engine::ComponentFactory::GetInstance().RegisterComponent<type>(name, mask);\
	}																	\
};																		\
																		\
static inline ComponentRegisterHelper __ourComponentRegisterHelper__;	\
																		\
type* Clone() const override {											\
	auto c = DBG_NEW type();											\
																		\
	type& this_ = const_cast<type&>(*this);								\
																		\
																		\
	c->myID = this_.myID;												\
																		\
	c->myGameObject = this_.myGameObject;								\
	c->OnConstruct();													\
																		\
	CopyComponentUsingReflect(&this_, c);								\
	return c;															\
}																		\
public:																	
												

#define COMPONENT_SINGLETON(type, name)									\
private:																\
class ComponentRegisterHelper											\
{																		\
public:																	\
	ComponentRegisterHelper()											\
	{																	\
		::Engine::ComponentFactory::GetInstance().RegisterComponent<type>(name, ::Engine::UPDATE);\
		::Engine::SingletonComponentFactory::GetInstance().RegisterSingletonComponent<type>(name);	\
	}																	\
};																		\
																		\
static inline ComponentRegisterHelper __ourComponentRegisterHelper__;	\
																		\
type* Clone() const override { \
	auto c = DBG_NEW type();\
\
	type& this_ = const_cast<type&>(*this);\
\
	\
	c->myID = this_.myID;\
\
	c->myGameObject = this_.myGameObject;\
	c->OnConstruct();\
\
	CopyComponentUsingReflect(&this_, c);\
	return c;\
}				\
public:																	
	

// A component that runs normal UPDATE, but
// uses the copy constructor instead of reflection to clone the component
// due to the reason that Physics components could not be used
// with reflect due to being broken and having to copy a PxTransform
// I just could not be bothered because it worked fine like this
#define COMPONENT_COPYABLE(type, name)									\
private:																\
class ComponentRegisterHelper											\
{																		\
public:																	\
	ComponentRegisterHelper()											\
	{																	\
		::Engine::ComponentFactory::GetInstance().RegisterComponent<type>(name, ::Engine::UPDATE);\
	}																	\
};																		\
																		\
static inline ComponentRegisterHelper __ourComponentRegisterHelper__;	\
																		\
type* Clone() const override { \
	auto c = DBG_NEW type(*this);\
c->OnConstruct();\
	return c;\
}				\
public:																	
	

#define COMPONENT_COPYABLE_MASK(type, name, mask)									\
private:																\
class ComponentRegisterHelper											\
{																		\
public:																	\
	ComponentRegisterHelper()											\
	{																	\
		::Engine::ComponentFactory::GetInstance().RegisterComponent<type>(name, mask);\
	}																	\
};																		\
																		\
static inline ComponentRegisterHelper __ourComponentRegisterHelper__;	\
																		\
type* Clone() const override { \
	auto c = DBG_NEW type(*this);\
c->OnConstruct();\
	return c;\
}				\
public:																	

class Component;

namespace Engine
{
	struct RegisteredComponentStruct
	{
		// choose at runtime, not static!
		int myComponentTypeId = -1;

		std::string myName;
		// TODO: Add a hash here
		Component* myComponent = nullptr;

		unsigned int mask = UPDATE;
		// only for debugging
		std::string myClassName;
	};

	class ComponentFactory
	{
	public:
		ComponentFactory() = default;
		ComponentFactory(const ComponentFactory&) = delete;
		~ComponentFactory() = default;

		// NOTE(filip): this is an instance like this to ensure
		// that the constructor to myComponentNameHashToRegisteredComponent
		// has always run BEFORE actualling inserting anything into it
		// needed to support the component registering using a define
		static ComponentFactory& GetInstance()
		{
			static ComponentFactory factory;
			return factory;
		}

		template <typename T>
		uint16_t GetComponentId();

		void RegisterComponentInternal(const RegisteredComponentStruct& aComponent);
		RegisteredComponentStruct* GetRegisteredComponentFromId(const int aComponentTypeId);
		RegisteredComponentStruct* GetRegisteredComponentFromComponentNameHash(const size_t aHash);
		int GetRegisteredComponentsCount();
		const std::vector<RegisteredComponentStruct*>& GetRegisteredComponents();

		template <typename T>
		void RegisterComponent(const std::string& aName, unsigned int aMask = UPDATE)
		{
			int id = GetComponentId<T>();

			T* component = DBG_NEW T();
			component->myID = id;

			RegisteredComponentStruct regComponent;
			regComponent.myComponentTypeId = id;
			regComponent.myComponent = component;
			regComponent.myName = aName;
			regComponent.mask = aMask;
			regComponent.myClassName = typeid(T).name();

			RegisterComponentInternal(regComponent);
		}

	private:
		uint16_t globalComponentTypeCounter = 0;

		std::vector<RegisteredComponentStruct*> myRegisteredComponents;
		std::unordered_map<size_t, int> myComponentNameHashToRegisteredComponent;
	};

	template <typename T>
	uint16_t ComponentFactory::GetComponentId()
	{
		static uint16_t id = globalComponentTypeCounter++;
		return id;
	}
}
