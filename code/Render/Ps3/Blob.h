#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace render
	{

class T_DLLCLASS Blob
{
public:
	Blob();

	Blob(uint32_t size);

	Blob(uint32_t size, const void* data);

	Blob(const Blob& blob);

	virtual ~Blob();

	void* getData() const;

	uint32_t getSize() const;

	bool serialize(ISerializer& s);

	Blob& operator = (const Blob& blob);

private:
	uint32_t m_size;
	uint8_t* m_data;
};

	}
}

