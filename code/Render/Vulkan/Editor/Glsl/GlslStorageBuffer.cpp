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

bool GlslStorageBuffer::add(const std::wstring& elementName, GlslType elementType)
{
	m_elements.push_back({ elementName, elementType });
	return true;
}

	}
}