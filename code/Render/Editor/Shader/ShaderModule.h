#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class T_DLLCLASS ShaderModule : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_text;
};

}