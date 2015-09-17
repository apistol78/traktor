#ifndef traktor_spark_ShapeResource_H
#define traktor_spark_ShapeResource_H

#include "Core/Math/Aabb2.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

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

class IRenderSystem;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class Shape;

/*! \brief Physical shape resource.
 * \ingroup Spark
 */
class T_DLLCLASS ShapeResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< Shape > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, db::Instance* resourceInstance) const;

	virtual void serialize(ISerializer& s);

private:
	friend class ShapePipeline;

	resource::Id< render::Shader > m_shader;
	std::vector< uint8_t > m_parts;
	Aabb2 m_bounds;
};

	}
}

#endif	// traktor_spark_ShapeResource_H
