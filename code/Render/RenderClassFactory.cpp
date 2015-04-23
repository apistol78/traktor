#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"
#include "Render/ICubeTexture.h"
#include "Render/ISimpleTexture.h"
#include "Render/IVolumeTexture.h"
#include "Render/RenderClassFactory.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderClassFactory", 0, RenderClassFactory, IRuntimeClassFactory)

void RenderClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< ITexture > > classITexture = new AutoRuntimeClass< ITexture >();
	registrar->registerClass(classITexture);

	Ref< AutoRuntimeClass< ICubeTexture > > classICubeTexture = new AutoRuntimeClass< ICubeTexture >();
	classICubeTexture->addMethod("getWidth", &ICubeTexture::getWidth);
	classICubeTexture->addMethod("getHeight", &ICubeTexture::getHeight);
	classICubeTexture->addMethod("getDepth", &ICubeTexture::getDepth);
	registrar->registerClass(classICubeTexture);

	Ref< AutoRuntimeClass< ISimpleTexture > > classISimpleTexture = new AutoRuntimeClass< ISimpleTexture >();
	classISimpleTexture->addMethod("getWidth", &ISimpleTexture::getWidth);
	classISimpleTexture->addMethod("getHeight", &ISimpleTexture::getHeight);
	registrar->registerClass(classISimpleTexture);

	Ref< AutoRuntimeClass< IVolumeTexture > > classIVolumeTexture = new AutoRuntimeClass< IVolumeTexture >();
	classIVolumeTexture->addMethod("getWidth", &IVolumeTexture::getWidth);
	classIVolumeTexture->addMethod("getHeight", &IVolumeTexture::getHeight);
	classIVolumeTexture->addMethod("getDepth", &IVolumeTexture::getDepth);
	registrar->registerClass(classIVolumeTexture);

	Ref< AutoRuntimeClass< IRenderView > > classIRenderView = new AutoRuntimeClass< IRenderView >();
	classIRenderView->addMethod("close", &IRenderView::close);
	classIRenderView->addMethod("getWidth", &IRenderView::getWidth);
	classIRenderView->addMethod("getHeight", &IRenderView::getHeight);
	classIRenderView->addMethod("isActive", &IRenderView::isActive);
	classIRenderView->addMethod("isFullScreen", &IRenderView::isFullScreen);
	classIRenderView->addMethod("showCursor", &IRenderView::showCursor);
	classIRenderView->addMethod("hideCursor", &IRenderView::hideCursor);
	classIRenderView->addMethod("isCursorVisible", &IRenderView::isCursorVisible);
	classIRenderView->addMethod("setGamma", &IRenderView::setGamma);
	registrar->registerClass(classIRenderView);
}

	}
}
