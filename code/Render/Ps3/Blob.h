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

	Blob(size_t size);

	Blob(size_t size, const void* data);

	Blob(const Blob& blob);

	virtual ~Blob();

	void* getData() const;

	size_t getSize() const;

	bool serialize(ISerializer& s);

	Blob& operator = (const Blob& blob);

private:
	size_t m_size;
	uint8_t* m_data;
};

	}
}

#endif	// traktor_render_Blob_H
