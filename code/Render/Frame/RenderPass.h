#pragma once

#include <functional>
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"

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

class IRenderGraph;
class RenderContext;

/*! Render pass definition.
 * \ingroup Render
 */
class T_DLLCLASS RenderPass : public Object
{
    T_RTTI_CLASS;

public:
	class IHandler : public IRefCount
	{
	public:
		virtual void executeRenderPass(IRenderGraph* renderGraph, RenderContext* renderContext) = 0;
	};

	class LambdaHandler : public RefCountImpl< IHandler >
	{
	public:
	    typedef std::function< void(IRenderGraph*, RenderContext*) > fn_t;

		explicit LambdaHandler(fn_t fn);

		virtual void executeRenderPass(IRenderGraph* renderGraph, RenderContext* renderContext) override final;

	private:
		fn_t m_fn;
	};

	void addInput(const std::wstring& targetSetName, uint32_t targetColorIndex);

	uint32_t getInputCount() const;

	void setOutput(const std::wstring& targetSetName);

	void setHandler(IHandler* handler);

	void setHandler(const LambdaHandler::fn_t& handler)
	{
		setHandler(new LambdaHandler(handler));
	}

private:
	friend class RenderGraph;

	struct Input
	{
		std::wstring targetSetName;
		uint32_t targetColorIndex;
	};

	std::wstring m_passId;
	AlignedVector< Input > m_inputs;
	std::wstring m_output;
	Ref< IHandler > m_handler;

	explicit RenderPass(const std::wstring& passId);
};

	}
}
