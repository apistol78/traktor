#ifndef traktor_spark_ShapeResource_H
#define traktor_spark_ShapeResource_H

#include "Resource/Id.h"
#include "Spark/ICharacterResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spark
	{

class T_DLLCLASS ShapeResource : public ICharacterResource
{
	T_RTTI_CLASS;

public:
	virtual Ref< Character > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, db::Instance* resourceInstance) const;

	virtual void serialize(ISerializer& s);

private:
	friend class ShapePipeline;

	resource::Id< render::Shader > m_shader;
};

	}
}

#endif	// traktor_spark_ShapeResource_H
