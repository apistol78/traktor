#ifndef traktor_render_ProgramResource_H
#define traktor_render_ProgramResource_H

#include <vector>
#include "Core/Serialization/ISerializable.h"
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

/*! \brief Program resource base class.
 * \ingroup Render
 */
class T_DLLCLASS ProgramResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addTexture(const std::wstring& parameterName, const Guid& guid);

	const std::vector< std::pair< std::wstring, Guid > >& getTextures() const;

	virtual bool serialize(ISerializer& s);

private:
	friend class ShaderPipeline;
	friend class ShaderFactory;

	std::vector< std::pair< std::wstring, Guid > > m_textures;
};

	}
}

#endif	// traktor_render_ProgramResource_H
