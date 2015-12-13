#include "Editor/App/Splash.h"
#include "Ui/FloodLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/Image.h"

// Resources
//#include "Resources/Splash.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.Splash", Splash, ui::ToolForm)

bool Splash::create()
{
	if (!ui::ToolForm::create(0, L"Splash", 0, 0, ui::WsTop, new ui::FloodLayout()))
		return false;

	//Ref< ui::Bitmap > splash = ui::Bitmap::load(c_ResourceSplash, sizeof(c_ResourceSplash), L"png");
	//T_ASSERT (splash);

	//Ref< ui::Image > image = new ui::Image();
	//image->create(this, splash, false);

	//fit();
	//center();
	//show();

	//raise();
	//update(0, true);

	return true;
}

	}
}
