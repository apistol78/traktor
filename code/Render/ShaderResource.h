#ifndef traktor_render_ShaderResource_H
#define traktor_render_ShaderResource_H

#include <map>
#include <vector>
#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

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

/*! \brief Shader resource base class.
 * \ingroup Render
 *
 * A shader resource is an intermediate representation
 * of a generated and compiled shader.
 */
class T_DLLCLASS ShaderResource : public Serializable
{
	T_RTTI_CLASS(ShaderResource)

public:
	struct Combination
	{
		uint32_t parameterValue;
		Ref< Serializable > program;

		bool serialize(Serializer& s);
	};

	struct Technique
	{
		std::wstring name;
		uint32_t parameterMask;
		std::vector< Combination > combinations;

		bool serialize(Serializer& s);
	};

	const std::map< std::wstring, uint32_t >& getParameterBits() const;

	const std::vector< Technique >& getTechniques() const;

	virtual bool serialize(Serializer& s);

private:
	friend class ShaderPipeline;

	std::map< std::wstring, uint32_t > m_parameterBits;
	std::vector< Technique > m_techniques;
};

	}
}

#endif	// traktor_render_ShaderResource_H
