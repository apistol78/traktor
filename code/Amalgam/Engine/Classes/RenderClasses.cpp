#include "Amalgam/Engine/Classes/RenderClasses.h"
#include "Render/IRenderView.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{

void registerRenderClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< render::IRenderView > > classRenderView = new script::AutoScriptClass< render::IRenderView >();
	classRenderView->addMethod("close", &render::IRenderView::close);
	classRenderView->addMethod("getWidth", &render::IRenderView::getWidth);
	classRenderView->addMethod("getHeight", &render::IRenderView::getHeight);
	classRenderView->addMethod("isActive", &render::IRenderView::isActive);
	classRenderView->addMethod("isFullScreen", &render::IRenderView::isFullScreen);
	classRenderView->addMethod("showCursor", &render::IRenderView::showCursor);
	classRenderView->addMethod("hideCursor", &render::IRenderView::hideCursor);
	classRenderView->addMethod("isCursorVisible", &render::IRenderView::isCursorVisible);
	classRenderView->addMethod("setGamma", &render::IRenderView::setGamma);
	scriptManager->registerClass(classRenderView);
}

	}
}
