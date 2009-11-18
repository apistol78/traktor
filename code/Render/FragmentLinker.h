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
class External;

/*! \brief Shader fragment linker.
 * \ingroup Render
 *
 * The fragment linker merges shader graph fragments
 * into a single shader graph by merging named Input- and OutputPorts.
 */
class T_DLLCLASS FragmentLinker : public Object
{
	T_RTTI_CLASS;

public:
	struct FragmentReader
	{
		virtual ~FragmentReader() {}

		/*! \brief Read callback.
		 *
		 * Read fragments from user defined source,
		 * Will automatically be resolved.
		 */
		virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) = 0;
	};

	FragmentLinker();

	FragmentLinker(FragmentReader& fragmentReader);

	/*! \brief Resolve shader graph.
	 *
	 * Resolving External nodes within shader graph and replacing
	 * them with resolved shader graph fragment.
	 *
	 * \param shaderGraph Shader graph to resolve.
	 * \param fullResolve Perform full resolve, child external references as well.
	 * \return Resolved shader graph.
	 */
	Ref< ShaderGraph > resolve(const ShaderGraph* shaderGraph, bool fullResolve);

private:
	FragmentReader* m_fragmentReader;
};

	}
}

#endif	// traktor_render_FragmentLinker_H
