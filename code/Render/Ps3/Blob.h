#ifndef traktor_render_Blob_H
#define traktor_render_Blob_H

#include "Core/Config.h"

namespace traktor
{

class ISerializer;

	namespace render
	{

class Blob
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

#endif	// traktor_render_Blob_H
