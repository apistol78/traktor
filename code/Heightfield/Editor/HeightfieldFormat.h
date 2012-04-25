#ifndef traktor_hf_HeightfieldFormat_H
#define traktor_hf_HeightfieldFormat_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

class HeightfieldFormat : public Object
{
	T_RTTI_CLASS;

public:
	Ref< Heightfield > read(const Path& filePath, const Vector4& worldExtent, bool invertX, bool invertZ, uint32_t detailSkip) const;
};

	}
}

#endif	// traktor_hf_HeightfieldFormat_H
