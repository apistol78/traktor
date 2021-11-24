#pragma once

#include <list>
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/Editor/Texture/TextureOutput.h"

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

class T_DLLCLASS SequenceTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	std::list< Path > m_fileNames;
	TextureOutput m_output;
	float m_rate = 10.0f;

	virtual void serialize(ISerializer& s) override final;
};

	}
}

