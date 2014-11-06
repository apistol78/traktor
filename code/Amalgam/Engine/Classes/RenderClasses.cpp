#include "Amalgam/Engine/Classes/RenderClasses.h"
#include "Render/IRenderView.h"
#include "Render/ITexture.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{

void registerRenderClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< render::ITexture > > classITexture = new script::AutoScriptClass< render::ITexture >();
	scriptManager->registerClass(classITexture);

	Ref< script::AutoScriptClass< render::IRenderView > > classIRenderView = new script::AutoScriptClass< render::IRenderView >();
	classIRenderView->addMethod("close", &render::IRenderView::close);
	classIRenderView->addMethod("getWidth", &render::IRenderView::getWidth);
	classIRenderView->addMethod("getHeight", &render::IRenderView::getHeight);
	classIRenderView->addMethod("isActive", &render::IRenderView::isActive);
	classIRenderView->addMethod("isFullScreen", &render::IRenderView::isFullScreen);
	classIRenderView->addMethod("showCursor", &render::IRenderView::showCursor);
	classIRenderView->addMethod("hideCursor", &render::IRenderView::hideCursor);
	classIRenderView->addMethod("isCursorVisible", &render::IRenderView::isCursorVisible);
	classIRenderView->addMethod("setGamma", &render::IRenderView::setGamma);
	scriptManager->registerClass(classIRenderView);
}

	}
}
