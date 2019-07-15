#pragma once

#include "Render/OpenGL/Std/Editor/Glsl/GlslResource.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslType.h"

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
		GlslType type;
	};

	GlslStorageBuffer(const std::wstring& name);

	bool add(const std::wstring& elementName, GlslType elementType);

	const AlignedVector< Element >& get() const { return m_elements; }

private:
	AlignedVector< Element > m_elements;
};

	}
}