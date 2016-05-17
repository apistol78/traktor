#ifndef traktor_spray_PointSetAsset_H
#define traktor_spray_PointSetAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

class T_DLLCLASS PointSetAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	PointSetAsset();

	bool fromFaces() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	bool m_fromFaces;
};

	}
}

#endif	// traktor_spray_PointSetAsset_H
