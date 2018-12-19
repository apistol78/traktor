#pragma once

#include "Core/Ref.h"
#include "Illuminate/Editor/IProbe.h"

namespace traktor
{
	namespace render
	{

class CubeMap;

	}

	namespace illuminate
	{

class CubeProbe : public IProbe
{
	T_RTTI_CLASS;

public:
	CubeProbe(const render::CubeMap* cubeMap);

	virtual Color4f sample(const Vector4& direction) const T_OVERRIDE T_FINAL;

private:
	Ref< const render::CubeMap > m_cubeMap;
};

	}
}
