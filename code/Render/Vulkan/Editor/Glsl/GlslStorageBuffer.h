#pragma once

#include "Render/Types.h"
#include "Render/Vulkan/Editor/Glsl/GlslResource.h"

namespace traktor
{
	namespace render
	{
	
class GlslStorageBuffer : public GlslResource
{
	T_RTTI_CLASS;

public:
	struct Element
	{
		std::wstring name;
		DataType type;
	};

	GlslStorageBuffer(const std::wstring& name);

	bool add(const std::wstring& elementName, DataType elementType);

	const AlignedVector< Element >& get() const { return m_elements; }

private:
	AlignedVector< Element > m_elements;
};

	}
}