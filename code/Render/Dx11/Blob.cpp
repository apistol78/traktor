#include <cstring>
#include "Render/Dx11//Blob.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Blob", Blob, Object)

Blob::Blob()
:	m_size(0)
{
}

Ref< Blob > Blob::create(uint32_t size)
{
	Ref< Blob > b = new Blob();
	b->m_data.reset((uint8_t*)getAllocator()->alloc(size, 16, T_FILE_LINE));
	b->m_size = size;
	return b;
}

Ref< Blob > Blob::create(const void* data, uint32_t size)
{
	Ref< Blob > b = create(size);
	std::memcpy(b->m_data.ptr(), data, size);
	return b;
}

	}
}
