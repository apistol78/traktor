#ifndef traktor_flash_FlashMovieLoader
#define traktor_flash_FlashMovieLoader

#include "Flash/IFlashMovieLoader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Job;

	namespace flash
	{

class T_DLLCLASS FlashMovieLoader : public IFlashMovieLoader
{
	T_RTTI_CLASS;

public:
	FlashMovieLoader();

	void setCacheDirectory(const std::wstring& cacheDirectory);

	void setMerge(bool merge);

	void setTriangulate(bool triangulate);

	void setIncludeAS(bool includeAS);

	virtual Ref< IHandle > loadAsync(const std::wstring& url) const T_OVERRIDE T_FINAL;

	virtual Ref< FlashMovie > load(const std::wstring& url) const T_OVERRIDE T_FINAL;

private:
	std::wstring m_cacheDirectory;
	bool m_merge;
	bool m_triangulate;
	bool m_includeAS;
};

	}
}

#endif	// traktor_flash_FlashMovieLoader
