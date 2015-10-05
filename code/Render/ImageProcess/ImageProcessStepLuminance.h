#ifndef traktor_render_ImageProcessStepLuminance_H
#define traktor_render_ImageProcessStepLuminance_H

#include "Core/Math/Vector4.h"
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

class Shader;

/*! \brief Luminance step.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepLuminance : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceLuminance : public Instance
	{
	public:
		InstanceLuminance(
			const ImageProcessStepLuminance* step,
			const resource::Proxy< Shader >& shader,
			handle_t source,
			const Vector4 sampleOffsets[16]
		);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		Ref< const ImageProcessStepLuminance > m_step;
		resource::Proxy< Shader > m_shader;
		handle_t m_source;
		handle_t m_handleSourceTexture;
		handle_t m_handleSampleOffsets;
		handle_t m_handleSampleOffsetScale;
		Vector4 m_sampleOffsets[16];
	};

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< Shader >& getShader() const { return m_shader; }

	const std::wstring& getSource() const { return m_source; }

private:
	resource::Id< Shader > m_shader;
	std::wstring m_source;
};

	}
}

#endif	// traktor_render_ImageProcessStepLuminance_H
