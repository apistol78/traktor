#pragma once

#include "Core/Ref.h"
#include "Resource/Id.h"
#include "Shape/Spline/LayerComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
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

	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS ExtrudeShapeLayerData : public LayerComponentData
{
	T_RTTI_CLASS;

public:
	ExtrudeShapeLayerData();

	virtual Ref< LayerComponent > createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	bool m_automaticOrientation;
	float m_detail;
};

	}
}
