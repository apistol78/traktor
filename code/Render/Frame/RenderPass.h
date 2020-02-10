#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
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
		bool history;			//!< Read previous frame; useful when cyclic behaviour desired.

		Input()
		:	targetSetId(0)
		,	colorIndex(0)
		,	history(false)
		{
		}
	};

	struct Output
	{
		handle_t targetSetId;	//!< Resource name.
		int32_t colorIndex;		//!< Index of target's color attachment, -1 means all attachments.
		Clear clear;			//!< Target clear value and mask.

		Output()
		:	targetSetId(0)
		,	colorIndex(-1)
		{
			clear.mask = 0;
		}
	};

	explicit RenderPass(const wchar_t* const name = L"Unnamed");

	const wchar_t* getName() const { return m_name; }

	//! \{

	void addInput(handle_t targetSetId);

	void addInput(handle_t targetSetId, int32_t colorIndex, bool history);

	AlignedVector< Input > getInputs() const;

	//! \}

	//! \{

	void setOutput(handle_t targetSetId);

	void setOutput(handle_t targetSetId, const Clear& clear);

	void setOutput(handle_t targetSetId, int32_t colorIndex, const Clear& clear);

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
	const wchar_t* m_name;
	AlignedVector< Input > m_inputs;
	Output m_output;
    AlignedVector< fn_build_t > m_builds;
	RefArray< const RenderPass > m_subPasses;
};

	}
}
