#ifndef traktor_flash_FlashMovieAsset_H
#define traktor_flash_FlashMovieAsset_H

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
	virtual void serialize(ISerializer& s);

private:
	friend class FlashPipeline;

	std::vector< Guid > m_dependentMovies;
};

	}
}

#endif	// traktor_flash_FlashMovieAsset_H
