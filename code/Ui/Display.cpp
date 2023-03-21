#include "Ui/Application.h"
#include "Ui/Display.h"

namespace traktor::ui
{

int32_t getSystemDPI()
{
	return Application::getInstance()->getWidgetFactory()->getSystemDPI();
}

}
