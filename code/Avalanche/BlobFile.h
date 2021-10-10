#pragma once

#include "Avalanche/IBlob.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AVALANCHE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace avalanche
	{

class T_DLLCLASS BlobFile : public IBlob
{
	T_RTTI_CLASS;

public:
	explicit BlobFile(const Path& path, int64_t size);

	virtual int64_t size() const override final;

	virtual Ref< IStream > append() override final;

	virtual Ref< IStream > read() const override final;

private:
    Path m_path;
	int32_t m_size;
};

	}
}
