#ifndef traktor_hf_MaterialMaskAssetLayer_H
#define traktor_hf_MaterialMaskAssetLayer_H

#include "Core/Math/Color4ub.h"
#include "Core/Serialization/ISerializable.h"

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

class T_DLLCLASS MaterialMaskAssetLayer : public ISerializable
{
	T_RTTI_CLASS;

public:
	const Color4ub& getColor() const;

	const RefArray< ISerializable >& getParams() const;

	virtual void serialize(ISerializer& s);

private:
	Color4ub m_color;
	RefArray< ISerializable > m_params;
};

	}
}

#endif	// traktor_hf_MaterialMaskAssetLayer_H

