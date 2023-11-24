#pragma once

#include <functional>
#include <string>
#include "Core/Guid.h"
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
	ShaderModule() = default;

	explicit ShaderModule(const std::wstring& text);

	void setTextDirect(const std::wstring& text);

	std::wstring escape(std::function< std::wstring (const Guid& g) > fn) const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_text;
};

}