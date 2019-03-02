#pragma once

#include "Flash/IMovieLoader.h"

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

class T_DLLCLASS MovieLoader : public IMovieLoader
{
	T_RTTI_CLASS;

public:
	MovieLoader();

	void setCacheDirectory(const std::wstring& cacheDirectory);

	void setMerge(bool merge);

	void setTriangulate(bool triangulate);

	void setIncludeAS(bool includeAS);

	virtual Ref< IHandle > loadAsync(const std::wstring& url) const override final;

	virtual Ref< Movie > load(const std::wstring& url) const override final;

private:
	std::wstring m_cacheDirectory;
	bool m_merge;
	bool m_triangulate;
	bool m_includeAS;
};

	}
}

