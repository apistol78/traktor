#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

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

class ShaderGraph;

/*! \brief Static analysis on shader graphs.
 * \ingroup Render
 */
class T_DLLCLASS ShaderGraphStatic : public Object
{
	T_RTTI_CLASS;

public:
	enum VariableResolveType
	{
		VrtLocal,
		VrtGlobal
	};

	ShaderGraphStatic(const ShaderGraph* shaderGraph);

	/*! \brief Get permutation of shader graph for given platform. */
	Ref< ShaderGraph > getPlatformPermutation(const std::wstring& platform) const;

	/*! \brief Get permutation of shader graph for given renderer. */
	Ref< ShaderGraph > getRendererPermutation(const std::wstring& renderer) const;

	/*! \brief Replace all "Connected" nodes with direct connections. */
	Ref< ShaderGraph > getConnectedPermutation() const;

	/*! \brief Replace all "Type" nodes with direct connections based on input type. */
	Ref< ShaderGraph > getTypePermutation() const;

	/*! \brief Insert swizzle nodes for all inputs to ensure widths are as small as possible. */
	Ref< ShaderGraph > getSwizzledPermutation() const;

	/*! \brief Calculate constant branches and replace with simpler branches. */
	Ref< ShaderGraph > getConstantFolded() const;

	/*! \brief Remove redundant swizzle nodes. */
	Ref< ShaderGraph > cleanupRedundantSwizzles() const;

	/*! \brief Propagate state given as input into PixelOutput. */
	Ref< ShaderGraph > getStateResolved() const;

	/*! \brief Replace variable nodes with direct connections. */
	Ref< ShaderGraph > getVariableResolved(VariableResolveType resolve) const;

	/*! \brief Remove disabled outputs. */
	Ref< ShaderGraph > removeDisabledOutputs() const;

private:
	Ref< const ShaderGraph > m_shaderGraph;
	Ref< const ShaderGraph > m_shaderGraphClean;
};

	}
}
