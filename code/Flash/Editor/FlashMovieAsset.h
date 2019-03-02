#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class T_DLLCLASS FlashMovieAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	FlashMovieAsset();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FlashPipeline;

	bool m_staticMovie;
	bool m_includeAS;
};

	}
}

