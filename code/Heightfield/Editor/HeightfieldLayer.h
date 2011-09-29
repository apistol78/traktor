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
	static Ref< HeightfieldLayer > createFromImage(drawing::Image* image);

	static Ref< HeightfieldLayer > createEmpty(uint32_t size, height_t height);

	uint32_t getSize() const { return m_size; }

	height_t* getHeights() { return m_heights.ptr(); }

	const height_t* getHeights() const { return m_heights.c_ptr(); }

private:
	uint32_t m_size;
	AutoArrayPtr< height_t > m_heights;

	HeightfieldLayer();
};

	}
}

#endif	// traktor_hf_HeightfieldLayer_H
