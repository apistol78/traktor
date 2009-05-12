#ifndef traktor_render_TextureLinker_H
#define traktor_render_TextureLinker_H

#include "Core/Object.h"
#include "Core/Guid.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ProgramResource;
class Program;
class Texture;

/*! \brief Bind textures to shaders.
 * \ingroup Render
 */
class T_DLLCLASS TextureLinker : public Object
{
	T_RTTI_CLASS(TextureLinker)

public:
	struct TextureReader
	{
		virtual Texture* read(const Guid& textureGuid) = 0;
	};

	TextureLinker(TextureReader& textureReader);

	bool link(const ProgramResource* programResource, Program* program);

private:
	TextureReader& m_textureReader;
};

	}
}

#endif	// traktor_render_TextureLinker_H
