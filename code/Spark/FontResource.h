#ifndef traktor_spark_FontResource_H
#define traktor_spark_FontResource_H

#include <vector>
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ITexture;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace spark
	{

class Font;

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS FontResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Glyph
	{
		uint32_t ch;
		uint32_t rect[4];

		Glyph();

		void serialize(ISerializer& s);
	};

	Ref< Font > create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s);

private:
	friend class FontPipeline;

	resource::Id< render::ITexture > m_texture;
	resource::Id< render::Shader > m_shader;
	std::vector< Glyph > m_glyphs;
	
};

	}
}

#endif	// traktor_spark_FontResource_H
