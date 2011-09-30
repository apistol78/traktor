#ifndef traktor_hf_HeightfieldLayer_H
#define traktor_hf_HeightfieldLayer_H

#include "Core/Object.h"
#include "Core/Misc/AutoPtr.h"
#include "Heightfield/HeightfieldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
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

	namespace hf
	{

class T_DLLCLASS HeightfieldLayer : public Object
{
	T_RTTI_CLASS;

public:
	drawing::Image* getImage() { return m_image; }

	const drawing::Image* getImage() const { return m_image; }

private:
	friend class HeightfieldCompositor;

	Ref< drawing::Image > m_image;

	HeightfieldLayer(drawing::Image* image);
};

	}
}

#endif	// traktor_hf_HeightfieldLayer_H
