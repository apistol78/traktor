/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_IMovieLoader
#define traktor_flash_IMovieLoader

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Net/Url.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Movie;

class T_DLLCLASS IMovieLoader : public Object
{
	T_RTTI_CLASS;

public:
	class IHandle : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual bool wait() = 0;

		virtual bool ready() = 0;

		virtual bool succeeded() = 0;

		virtual Ref< Movie > get() = 0;

		bool failed() { return !succeeded(); }
	};

	virtual Ref< IHandle > loadAsync(const std::wstring& url) const = 0;

	virtual Ref< Movie > load(const std::wstring& url) const = 0;
};

	}
}

#endif	// traktor_flash_IMovieLoader
