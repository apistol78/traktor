#include "Ui/Custom/ColorPicker/ColorSliderControl.h"
#include "Ui/Custom/ColorPicker/ColorEvent.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sliderWidth = 24;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorSliderControl", ColorSliderControl, Widget)

bool ColorSliderControl::create(Widget* parent, int style, IGradient* gradient)
{
	if (!Widget::create(parent, style))
		return false;

	addButtonDownEventHandler(createMethodHandler(this, &ColorSliderControl::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &ColorSliderControl::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &ColorSliderControl::eventMouseMove));
	addPaintEventHandler(createMethodHandler(this, &ColorSliderControl::eventPaint));

	m_gradient = gradient;
	m_gradientBitmap = new Bitmap(c_sliderWidth, 256);

	updateGradient();
	update();

	return true;
}

Size ColorSliderControl::getPreferedSize() const
{
	return Size(c_sliderWidth, 256);
}

void ColorSliderControl::addColorSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiColorSelect, eventHandler);
}

void ColorSliderControl::updateGradient()
{
	for (int y = 0; y < 256; ++y)
	{
		Color color = m_gradient->get(y);
		for (int x = 0; x < c_sliderWidth; ++x)
		{
			Color checkerColor = (((x >> 2) & 1) ^ ((y >> 2) & 1)) ? Color(180, 180, 180) : Color(80, 80, 80);
			Color gradientColor = lerp(checkerColor, color, color.a / 255.0f);
			m_gradientBitmap->setPixel(x, y, gradientColor);
		}
	}
}

void ColorSliderControl::eventButtonDown(Event* event)
{
	int y = checked_type_cast< MouseEvent* >(event)->getPosition().y;
	T_ASSERT (y >= 0 && y < 256);

	Color color = m_gradient->get(y);

	ColorEvent colorEvent(this, 0, color);
	raiseEvent(EiColorSelect, &colorEvent);
	setCapture();
}

void ColorSliderControl::eventButtonUp(Event* event)
{
	releaseCapture();
}

void ColorSliderControl::eventMouseMove(Event* event)
{
	if (!hasCapture())
		return;

	int y = checked_type_cast< MouseEvent* >(event)->getPosition().y;
	if (y < 0)
		y = 0;
	if (y > 255)
		y = 255;

	Color color = m_gradient->get(y);

	ColorEvent colorEvent(this, 0, color);
	raiseEvent(EiColorSelect, &colorEvent);
}

void ColorSliderControl::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		Size(c_sliderWidth, 256),
		m_gradientBitmap
	);

	paintEvent->consume();
}

		}
	}
}
