#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class T_DLLCLASS MovieAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	MovieAsset();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class Pipeline;

	bool m_staticMovie;
};

	}
}

