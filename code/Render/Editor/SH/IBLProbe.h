#pragma once

#include "Render/SH/SHFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace render
	{

/*!
 * \brief "Image based lighting" probe.
 */
class T_DLLCLASS IBLProbe : public SHFunction
{
	T_RTTI_CLASS;

public:
	IBLProbe(drawing::Image* image);

	virtual Vector4 evaluate(float phi, float theta, const Vector4& unit) const override final;

private:
	Ref< drawing::Image > m_image;
};

	}
}
