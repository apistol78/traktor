#include "Core/Misc/Adler32.h"
#include "Render/Vulkan/Editor/Glsl/GlslStorageBuffer.h"

namespace traktor
{
	namespace render
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.render.GlslStorageBuffer", GlslStorageBuffer, GlslResource)

GlslStorageBuffer::GlslStorageBuffer(const std::wstring& name)
:	GlslResource(name)
{
}

bool GlslStorageBuffer::add(const std::wstring& elementName, DataType elementType)
{
	m_elements.push_back({ elementName, elementType });
	return true;
}

int32_t GlslStorageBuffer::getOrdinal() const
{
	Adler32 cs;
	cs.begin();
	cs.feed(getName());
	cs.end();
	return (int32_t)cs.get();
}

	}
}