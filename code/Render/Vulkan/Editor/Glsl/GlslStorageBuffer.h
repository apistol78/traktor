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

	explicit GlslStorageBuffer(const std::wstring& name, uint8_t stages);

	bool add(const std::wstring& elementName, DataType elementType);

	const AlignedVector< Element >& get() const { return m_elements; }

	virtual int32_t getOrdinal() const override final;

private:
	AlignedVector< Element > m_elements;
};

	}
}