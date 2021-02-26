#pragma once

#include "Core/Ref.h"
#include "Spark/Bitmap.h"
#include "Spark/SwfTypes.h"

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

class ISimpleTexture;

	}

	namespace spark
	{

/*! Bitmap texture container.
 * \ingroup Spark
 */
class T_DLLCLASS BitmapTexture : public Bitmap
{
	T_RTTI_CLASS;

public:
	BitmapTexture() = default;

	explicit BitmapTexture(render::ISimpleTexture* texture);

	render::ISimpleTexture* getTexture() const { return m_texture; }

private:
	Ref< render::ISimpleTexture > m_texture;
};

	}
}

