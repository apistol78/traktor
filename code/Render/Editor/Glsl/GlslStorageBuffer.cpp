#include "Core/Misc/Murmur3.h"
#include "Render/Editor/Glsl/GlslStorageBuffer.h"

namespace traktor::render
{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslStorageBuffer", GlslStorageBuffer, GlslResource)

GlslStorageBuffer::GlslStorageBuffer(const std::wstring& name, uint8_t stages)
:	GlslResource(name, stages)
{
}

bool GlslStorageBuffer::add(const std::wstring& elementName, DataType elementType)
{
	m_elements.push_back({ elementName, elementType });
	return true;
}

int32_t GlslStorageBuffer::getOrdinal() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

}
