#pragma once

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
	
class Button;
class ProgressBar;
class Static;

	}

	namespace shape
	{

class TracerPanel : public ui::Container
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	virtual void destroy() override final;

private:
	Ref< ui::Static > m_staticDescription;
	Ref< ui::ProgressBar > m_progressBar;
	Ref< ui::Button > m_buttonAbort;
};

	}
}
