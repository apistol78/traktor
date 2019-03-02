#pragma once

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

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_fromFaces;
};

	}
}

