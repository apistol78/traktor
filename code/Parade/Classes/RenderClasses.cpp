#include "Parade/Classes/RenderClasses.h"
#include "Render/IRenderView.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{

void registerRenderClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< render::IRenderView > > classRenderView = new script::AutoScriptClass< render::IRenderView >();
	classRenderView->addMethod(L"close", &render::IRenderView::close);
	classRenderView->addMethod(L"getWidth", &render::IRenderView::getWidth);
	classRenderView->addMethod(L"getHeight", &render::IRenderView::getHeight);
	classRenderView->addMethod(L"isActive", &render::IRenderView::isActive);
	classRenderView->addMethod(L"isFullScreen", &render::IRenderView::isFullScreen);
	classRenderView->addMethod(L"showCursor", &render::IRenderView::showCursor);
	classRenderView->addMethod(L"hideCursor", &render::IRenderView::hideCursor);
	classRenderView->addMethod(L"setGamma", &render::IRenderView::setGamma);
	scriptManager->registerClass(classRenderView);
}

	}
}
