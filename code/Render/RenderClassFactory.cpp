#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"
#include "Render/ICubeTexture.h"
#include "Render/ISimpleTexture.h"
#include "Render/IVolumeTexture.h"
#include "Render/RenderClassFactory.h"
#include "Render/ImageProcess/ImageProcess.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class BoxedDisplayMode : public Object
{
	T_RTTI_CLASS;

public:
	BoxedDisplayMode(const DisplayMode& displayMode)
	:	m_displayMode(displayMode)
	{
	}

	uint32_t getWidth() const { return m_displayMode.width; }

	uint32_t getHeight() const { return m_displayMode.height; }

	uint16_t getRefreshRate() const { return m_displayMode.refreshRate; }

	uint16_t getColorBits() const { return m_displayMode.colorBits; }

private:
	DisplayMode m_displayMode;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DisplayMode", BoxedDisplayMode, Object)

Ref< BoxedDisplayMode > IRenderSystem_getDisplayMode(IRenderSystem* this_, uint32_t index)
{
	return new BoxedDisplayMode(this_->getDisplayMode(index));
}

Ref< BoxedDisplayMode > IRenderSystem_getCurrentDisplayMode(IRenderSystem* this_)
{
	return new BoxedDisplayMode(this_->getCurrentDisplayMode());
}

void ImageProcess_setCombination(ImageProcess* this_, const std::wstring& name, bool value)
{
	this_->setCombination(render::getParameterHandle(name), value);
}

void ImageProcess_setParameter(ImageProcess* this_, const std::wstring& name, float value)
{
	this_->setFloatParameter(render::getParameterHandle(name), value);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderClassFactory", 0, RenderClassFactory, IRuntimeClassFactory)

void RenderClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< BoxedDisplayMode > > classBoxedDisplayMode = new AutoRuntimeClass< BoxedDisplayMode >();
	classBoxedDisplayMode->addProperty("width", &BoxedDisplayMode::getWidth);
	classBoxedDisplayMode->addProperty("height", &BoxedDisplayMode::getHeight);
	classBoxedDisplayMode->addProperty("refreshRate", &BoxedDisplayMode::getRefreshRate);
	classBoxedDisplayMode->addProperty("colorBits", &BoxedDisplayMode::getColorBits);
	registrar->registerClass(classBoxedDisplayMode);

	Ref< AutoRuntimeClass< ITexture > > classITexture = new AutoRuntimeClass< ITexture >();
	classITexture->addProperty("mips", &ITexture::getMips);
	registrar->registerClass(classITexture);

	Ref< AutoRuntimeClass< ICubeTexture > > classICubeTexture = new AutoRuntimeClass< ICubeTexture >();
	classICubeTexture->addProperty("side", &ICubeTexture::getSide);
	registrar->registerClass(classICubeTexture);

	Ref< AutoRuntimeClass< ISimpleTexture > > classISimpleTexture = new AutoRuntimeClass< ISimpleTexture >();
	classISimpleTexture->addProperty("width", &ISimpleTexture::getWidth);
	classISimpleTexture->addProperty("height", &ISimpleTexture::getHeight);
	registrar->registerClass(classISimpleTexture);

	Ref< AutoRuntimeClass< IVolumeTexture > > classIVolumeTexture = new AutoRuntimeClass< IVolumeTexture >();
	classIVolumeTexture->addProperty("width", &IVolumeTexture::getWidth);
	classIVolumeTexture->addProperty("height", &IVolumeTexture::getHeight);
	classIVolumeTexture->addProperty("depth", &IVolumeTexture::getDepth);
	registrar->registerClass(classIVolumeTexture);

	Ref< AutoRuntimeClass< IRenderSystem > > classIRenderSystem = new AutoRuntimeClass< IRenderSystem >();
	classIRenderSystem->addProperty("displayModeCount", &IRenderSystem::getDisplayModeCount);
	classIRenderSystem->addProperty("currentDisplayMode", &IRenderSystem_getCurrentDisplayMode);
	classIRenderSystem->addProperty("displayAspectRatio", &IRenderSystem::getDisplayAspectRatio);
	classIRenderSystem->addMethod("getDisplayMode", &IRenderSystem_getDisplayMode);
	registrar->registerClass(classIRenderSystem);

	Ref< AutoRuntimeClass< IRenderView > > classIRenderView = new AutoRuntimeClass< IRenderView >();
	classIRenderView->addProperty("width", &IRenderView::getWidth);
	classIRenderView->addProperty("height", &IRenderView::getHeight);
	classIRenderView->addProperty("isActive", &IRenderView::isActive);
	classIRenderView->addProperty("isMinimized", &IRenderView::isMinimized);
	classIRenderView->addProperty("isFullScreen", &IRenderView::isFullScreen);
	classIRenderView->addMethod("close", &IRenderView::close);
	classIRenderView->addMethod("showCursor", &IRenderView::showCursor);
	classIRenderView->addMethod("hideCursor", &IRenderView::hideCursor);
	classIRenderView->addMethod("isCursorVisible", &IRenderView::isCursorVisible);
	classIRenderView->addMethod("setGamma", &IRenderView::setGamma);
	registrar->registerClass(classIRenderView);

	Ref< AutoRuntimeClass< ImageProcess > > classImageProcess = new AutoRuntimeClass< ImageProcess >();
	classImageProcess->addMethod("setCombination", &ImageProcess_setCombination);
	classImageProcess->addMethod("setParameter", &ImageProcess_setParameter);
	classImageProcess->addMethod("requireHighRange", &ImageProcess::requireHighRange);
	registrar->registerClass(classImageProcess);
}

	}
}
