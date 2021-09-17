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

class ChunkMemory;
class IStream;

	namespace avalanche
	{

class T_DLLCLASS Blob : public Object
{
	T_RTTI_CLASS;

public:
	Blob();

	int64_t size() const;

	Ref< IStream > append(int64_t appendSize);

	Ref< IStream > read() const;

private:
	Ref< ChunkMemory > m_memory;
};

	}
}
