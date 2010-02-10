#ifndef traktor_render_ShaderResource_H
#define traktor_render_ShaderResource_H

#include <map>
#include <vector>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Shader resource base class.
 * \ingroup Render
 *
 * A shader resource is an intermediate representation
 * of a generated and compiled shader.
 */
class T_DLLCLASS ShaderResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Combination
	{
		uint32_t parameterValue;
		Ref< ISerializable > program;

		bool serialize(ISerializer& s);
	};

	struct Technique
	{
		std::wstring name;
		uint32_t parameterMask;
		std::vector< Combination > combinations;

		bool serialize(ISerializer& s);
	};

	const std::map< std::wstring, uint32_t >& getParameterBits() const;

	const std::vector< Technique >& getTechniques() const;

	void addTexture(const Guid& guid);

	const std::vector< Guid >& getTextures() const;

	virtual bool serialize(ISerializer& s);

private:
	friend class ShaderPipeline;

	std::map< std::wstring, uint32_t > m_parameterBits;
	std::vector< Technique > m_techniques;
	std::vector< Guid > m_textures;
};

	}
}

#endif	// traktor_render_ShaderResource_H
