#pragma once

#include "Avalanche/IBlob.h"

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

	namespace avalanche
	{

class T_DLLCLASS BlobMemory : public IBlob
{
	T_RTTI_CLASS;

public:
	BlobMemory();

	virtual int64_t size() const override final;

	virtual Ref< IStream > append() override final;

	virtual Ref< IStream > read() const override final;

	virtual bool remove() override final;

private:
	Ref< ChunkMemory > m_memory;
};

	}
}
