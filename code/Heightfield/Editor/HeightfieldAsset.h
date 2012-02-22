#ifndef traktor_hf_HeightfieldAsset_H
#define traktor_hf_HeightfieldAsset_H

#include "Core/Math/Vector4.h"
#include "Editor/Asset.h"

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

class T_DLLCLASS HeightfieldAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	HeightfieldAsset();

	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);

	const Vector4& getWorldExtent() const { return m_worldExtent; }

	uint32_t getPatchDim() const { return m_patchDim; }

	uint32_t getDetailSkip() const { return m_detailSkip; }

	bool getInvertX() const { return m_invertX; }

	bool getInvertZ() const { return m_invertZ; }

private:
	Vector4 m_worldExtent;
	uint32_t m_patchDim;
	uint32_t m_detailSkip;
	bool m_invertX;
	bool m_invertZ;
};

	}
}

#endif	// traktor_hf_HeightfieldAsset_H
