#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
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
class RenderPassBuilder;
class RenderPassResources;

/*! Render pass definition.
 * \ingroup Render
 */
class T_DLLCLASS RenderPass : public Object
{
	T_RTTI_CLASS;

public:
	RenderPass();

	typedef std::function< void(RenderPassBuilder&) > fn_setup_t;

	typedef std::function< void(RenderPassResources&, RenderContext*) > fn_build_t;

private:
	friend class RenderGraph;
	friend class RenderPassBuilder;
	friend class RenderPassResources;

	struct Input
	{
		handle_t name;		//!< Resource name.
		int32_t colorIndex;	//!< Index of target's color attachment.
		bool history;		//!< Read previous frame; useful when cyclic behaviour desired.

		Input()
		:	name(0)
		,	colorIndex(0)
		,	history(false)
		{
		}
	};

	struct Output
	{
		handle_t name;		//!< Resource name.
		int32_t colorIndex;	//!< Index of target's color attachment, -1 means all attachments.
		Clear clear;		//!< Target clear value and mask.

		Output()
		:	name(0)
		,	colorIndex(-1)
		{
			clear.mask = 0;
		}
	};

	const wchar_t* m_name;
	AlignedVector< Input > m_inputs;
	Output m_output;
	fn_build_t m_build;
	
	int32_t m_refs;
};

	}
}
