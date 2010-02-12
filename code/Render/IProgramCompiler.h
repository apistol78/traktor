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
	/*! \brief Compile program.
	 *
	 * \param shaderGraph Program shader graph.
	 * \param optimize Optimization level (0-4; 0 = No optimization; 4 = Maximum optimization).
	 * \param validate Validate compiled program.
	 * \param outCostEstimate Estimated cost; returned value is compiler dependent and thus only relative values are of interest.
	 * \return Compiled program resource.
	 */
	virtual Ref< ProgramResource > compile(
		const ShaderGraph* shaderGraph,
		int32_t optimize,
		bool validate,
		uint32_t* outCostEstimate = 0
	) const = 0;
};

	}
}

#endif	// traktor_render_IProgramCompiler_H
