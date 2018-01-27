/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessStepBokeh_H
#define traktor_render_ImageProcessStepBokeh_H

#include <vector>
#include "Core/Containers/AlignedVector.h"
#include "Render/Types.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Render/ImageProcess/ImageProcessStep.h"

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

class IndexBuffer;
class Shader;
class VertexBuffer;

/*! \brief Post bokeh step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepBokeh : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	struct Source
	{
		std::wstring param;		/*!< Shader parameter name. */
		std::wstring source;	/*!< Render target set source. */
		uint32_t index;			/*!< Render target index. */

		Source();

		void serialize(ISerializer& s);
	};

	class InstanceBokeh : public Instance
	{
	public:
		struct Source
		{
			handle_t param;
			handle_t source;
			uint32_t index;
		};

		InstanceBokeh(
			const resource::Proxy< Shader >& shader,
			VertexBuffer* vertexBuffer,
			IndexBuffer* indexBuffer,
			const std::vector< Source >& sources,
			uint32_t quadCount
		);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		resource::Proxy< Shader > m_shader;
		Ref< VertexBuffer > m_vertexBuffer;
		Ref< IndexBuffer > m_indexBuffer;
		std::vector< Source > m_sources;
		uint32_t m_quadCount;
		float m_time;
		handle_t m_handleTime;
		handle_t m_handleDeltaTime;
		handle_t m_handleRatio;
	};

	ImageProcessStepBokeh();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::vector< Source >& getSources() const { return m_sources; }

private:
	resource::Id< Shader > m_shader;
	std::vector< Source > m_sources;
};

	}
}

#endif	// traktor_render_ImageProcessStepBokeh_H
