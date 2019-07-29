#pragma once

#include "Core/Ref.h"
#include "Shape/Editor/Bake/IProbe.h"

namespace traktor
{
	namespace render
	{

class CubeMap;

	}

	namespace shape
	{
	
class IblProbe : public IProbe
{
	T_RTTI_CLASS;

public:
	explicit IblProbe(const render::CubeMap* cubeMap);

	virtual Color4f sample(const Vector4* directions, uint32_t count) const override final;

private:
	Ref< const render::CubeMap > m_cubeMap;
};
	
	}
}
