#ifndef QuadComponentData_H
#define QuadComponentData_H

#include <Render/IRenderSystem.h>
#include <Render/Shader.h>
#include <Resource/Id.h>
#include <Resource/IResourceManager.h>
#include <World/IEntityComponentData.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MYCPPTEST_SHARED_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

class QuadComponent;

class T_DLLCLASS QuadComponentData : public traktor::world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	traktor::Ref< QuadComponent > createComponent(traktor::resource::IResourceManager* resourceManager, traktor::render::IRenderSystem* renderSystem) const;

	virtual void serialize(traktor::ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class Pipeline;

	traktor::resource::Id< traktor::render::Shader > m_shader;
};

#endif	// QuadComponentData_H
