#pragma once

#include "Render/Editor/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS ImgInput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgInput();

	const std::wstring& getTextureId() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_textureId;
};

	}   
}