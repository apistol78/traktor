#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace avalanche
	{

class T_DLLCLASS IBlob : public Object
{
	T_RTTI_CLASS;

public:
	virtual int64_t size() const = 0;

	virtual Ref< IStream > append() = 0;

	virtual Ref< IStream > read() const = 0;

	virtual bool remove() = 0;
};

	}
}
