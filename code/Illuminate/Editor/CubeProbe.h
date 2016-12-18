#ifndef traktor_illuminate_CubeProbe_H
#define traktor_illuminate_CubeProbe_H

#include "Illuminate/Editor/IProbe.h"

namespace traktor
{
	namespace illuminate
	{

class CubeMap;

class CubeProbe : public IProbe
{
	T_RTTI_CLASS;

public:
	CubeProbe(const drawing::Image* cubeMap);

	virtual Color4f sample(const Vector4& direction) const T_OVERRIDE T_FINAL;

private:
	Ref< CubeMap > m_cubeMap;
};

	}
}

#endif	// traktor_illuminate_CubeProbe_H
