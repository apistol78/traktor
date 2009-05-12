#ifndef traktor_render_FragmentLinker_H
#define traktor_render_FragmentLinker_H

#include <string>
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

class ShaderGraph;

/*! \brief Shader fragment linker.
 * \ingroup Render
 *
 * The fragment linker merges shader graph fragments
 * into a single shader graph by merging named Input- and OutputPorts.
 */
class T_DLLCLASS FragmentLinker : public Object
{
	T_RTTI_CLASS(FragmentLinker)

public:
	struct FragmentReader
	{
		virtual ~FragmentReader() {}

		/*! \brief Read callback.
		 *
		 * Read fragments from user defined source,
		 * Will automatically be resolved.
		 */
		virtual const ShaderGraph* read(const Guid& fragmentGuid) = 0;
	};

	FragmentLinker();

	FragmentLinker(FragmentReader& fragmentReader);

	/*! \brief Resolve shader graph.
	 *
	 * Resolving External nodes within shader graph and replacing
	 * them with resolved shader graph fragment.
	 */
	ShaderGraph* resolve(const ShaderGraph* shaderGraph);

	/*! \brief Merge shader graph fragments.
	 *
	 * Graphs are merged left->right, i.e. output ports from left graph
	 * is connected to input ports in the right graph.
	 */
	ShaderGraph* merge(const ShaderGraph* shaderGraphLeft, const ShaderGraph* shaderGraphRight);

private:
	FragmentReader* m_fragmentReader;
};

	}
}

#endif	// traktor_render_FragmentLinker_H
