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

class T_DLLCLASS ImgOutput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ImgOutput();

	virtual void serialize(ISerializer& s) override final;
};

	}   
}