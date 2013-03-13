#ifndef traktor_hf_HeightfieldFormat_H
#define traktor_hf_HeightfieldFormat_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class Heightfield;

class T_DLLCLASS HeightfieldFormat : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Heightfield > read(IStream* stream, const std::wstring& extension, const Vector4& worldExtent, bool invertX, bool invertZ, uint32_t detailSkip) const;
};

	}
}

#endif	// traktor_hf_HeightfieldFormat_H
