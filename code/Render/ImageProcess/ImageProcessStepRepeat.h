/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ImageProcessStepRepeat_H
#define traktor_render_ImageProcessStepRepeat_H

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

/*! \brief Repeat step any number of times.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessStepRepeat : public ImageProcessStep
{
	T_RTTI_CLASS;

public:
	class InstanceRepeat : public Instance
	{
	public:
		InstanceRepeat(uint32_t count, Instance* instance);

		virtual void destroy() T_OVERRIDE T_FINAL;

		virtual void render(
			ImageProcess* imageProcess,
			IRenderView* renderView,
			ScreenRenderer* screenRenderer,
			const RenderParams& params
		) T_OVERRIDE T_FINAL;

	private:
		uint32_t m_count;
		Ref< Instance > m_instance;
	};

	ImageProcessStepRepeat();

	virtual Ref< Instance > create(
		resource::IResourceManager* resourceManager,
		IRenderSystem* renderSystem,
		uint32_t width,
		uint32_t height
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const Ref< ImageProcessStep >& getStep() const { return m_step; }

private:
	uint32_t m_count;
	Ref< ImageProcessStep > m_step;
};

	}
}

#endif	// traktor_render_ImageProcessStepRepeat_H
