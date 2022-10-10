#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IProgram;

/*! Shader
 * \ingroup Render
 *
 * An shader is actually only a facade for several
 * programs which is different combinations of a
 * source shader graph.
 */
class T_DLLCLASS Shader : public Object
{
	T_RTTI_CLASS;

public:
	/*! Shader permutation.
	 *
	 * A shader permutation is defined as
	 * being both technique and mask of
	 * enabled combinations.
	 * 
	 * Use Shader::setCombination to setup
	 * combination mask from handles.
	 */
	struct Permutation
	{
		handle_t technique;
		uint32_t combination;

		Permutation()
		:	technique(ms_default)
		,	combination(0)
		{
		}

		explicit Permutation(handle_t technique_)
		:	technique(technique_)
		,	combination(0)
		{
		}

		explicit Permutation(handle_t technique_, uint32_t combination_)
		:	technique(technique_)
		,	combination(combination_)
		{
		}
	};

	struct Program
	{
		IProgram* program;
		uint32_t priority;

		operator bool () const
		{
			return (bool)(program != nullptr);
		}
	};

	virtual ~Shader();

	void destroy();

	/*! Get set of shader techniques.
	 *
	 * \param outHandles Set of technique handles.
	 */
	void getTechniques(SmallSet< handle_t >& outHandles) const;

	/*! Check if shader support technique. */
	bool hasTechnique(handle_t handle) const;

	/*! Set combination mask.
	 *
	 * \param handle Branch parameter handle.
	 * \param param Branch path.
	 * \param inoutCombinationMask Combination mask to modify.
	 */
	void setCombination(handle_t handle, bool param, Permutation& inoutPermutation) const;

	/*! Get program and priority from technique and combination mask. */
	Program getProgram(const Permutation& permutation = Permutation()) const;

private:
	friend class ShaderFactory;

	struct Combination
	{
		uint32_t mask;
		uint32_t value;
		uint32_t priority;
		Ref< IProgram > program;
	};

	struct Technique
	{
		uint32_t mask;
		AlignedVector< Combination > combinations;
	};

	const static Handle ms_default;

	SmallMap< handle_t, Technique > m_techniques;
	SmallMap< handle_t, uint32_t > m_parameterBits;
};

}
