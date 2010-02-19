#ifndef traktor_render_IProgramCompiler_H
#define traktor_render_IProgramCompiler_H

#include "Core/Object.h"

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

class ProgramResource;
class ShaderGraph;

/*! \brief Program compiler interface.
 * \ingroup Render
 */
class T_DLLCLASS IProgramCompiler : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Compiled stats. */
	struct Stats
	{
		uint32_t vertexCost;
		uint32_t pixelCost;

		Stats()
		:	vertexCost(0)
		,	pixelCost(0)
		{
		}
	};

	/*! \brief Get platform signature.
	 *
	 * \return Platform signature.
	 */
	virtual const wchar_t* getPlatformSignature() const = 0;

	/*! \brief Compile program.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param optimize Optimization level (0-4; 0 = No optimization; 4 = Maximum optimization).
	 * \param validate Validate compiled program.
	 * \param outStats Optional stats.
	 * \return Compiled program resource.
	 */
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		Stats* outStats = 0
	) const = 0;
};

	}
}

#endif	// traktor_render_IProgramCompiler_H
