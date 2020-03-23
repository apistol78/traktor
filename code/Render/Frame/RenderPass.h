#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Types.h"

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

class RenderContext;
class RenderGraph;

/*! Render pass definition.
 * \ingroup Render
 */
class T_DLLCLASS RenderPass : public Object
{
	T_RTTI_CLASS;

public:
	typedef std::function< void(const RenderGraph&, RenderContext*) > fn_build_t;

	struct Input
	{
		handle_t targetSetId;	//!< Resource name.
		int32_t colorIndex;		//!< Index of target's color attachment.

		Input()
		:	targetSetId(0)
		,	colorIndex(0)
		{
		}
	};

	struct Output
	{
		handle_t targetSetId;	//!< Resource name, all set's color attachments are written to by pass.
		Clear clear;			//!< Target clear value and mask.

		Output()
		:	targetSetId(0)
		{
			clear.mask = 0;
		}
	};

	explicit RenderPass(const std::wstring& name = L"Unnamed");

	const std::wstring& getName() const { return m_name; }

	//! \{

	void addInput(handle_t targetSetId);

	StaticVector< Input, 16 > getInputs() const;

	//! \}

	//! \{

	void setOutput(handle_t targetSetId);

	void setOutput(handle_t targetSetId, const Clear& clear);

	bool haveOutput() const { return m_output.targetSetId != ~0; }

	const Output& getOutput() const { return m_output; }

	//! \}

	//! \{

	void addBuild(const fn_build_t& build);

	const AlignedVector< fn_build_t >& getBuilds() const { return m_builds; }

	//! \}

	//! \{

	void addSubPass(const RenderPass* subPass);

	const RefArray< const RenderPass >& getSubPasses() const { return m_subPasses; }

	//! \}

protected:
	std::wstring m_name;
	StaticVector< Input, 16 > m_inputs;
	Output m_output;
    AlignedVector< fn_build_t > m_builds;
	RefArray< const RenderPass > m_subPasses;
};

	}
}
