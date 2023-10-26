#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/ISerializer.h"
#include "Render/Editor/Shader/ShaderModule.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderModule", 0, ShaderModule, ISerializable)

void ShaderModule::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& ShaderModule::getText() const
{
	return m_text;
}

void ShaderModule::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"text", m_text, AttributeMultiLine());
}

}
