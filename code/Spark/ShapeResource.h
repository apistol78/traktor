#ifndef traktor_spark_ShapeResource_H
#define traktor_spark_ShapeResource_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
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
class ITexture;
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

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ImageShapePipeline;
	friend class VectorShapePipeline;

	struct Part
	{
		resource::Id< render::ITexture > texture;
		Color4f fillColor;
		int8_t curveSign;

		void serialize(ISerializer& s);
	};

	resource::Id< render::Shader > m_shader;
	AlignedVector< Part > m_parts;
	Aabb2 m_bounds;
};

	}
}

#endif	// traktor_spark_ShapeResource_H
