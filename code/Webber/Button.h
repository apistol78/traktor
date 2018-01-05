#ifndef traktor_wbr_Button_H
#define traktor_wbr_Button_H

#include "Webber/Event.h"
#include "Webber/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace wbr
	{

class T_DLLCLASS Button : public Widget
{
	T_RTTI_CLASS;

public:
	Button(const std::wstring& text);

	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;

	Event& clicked();

private:
	std::wstring m_text;
	Event m_clicked;
};
		
	}
}

#endif	// traktor_wbr_Button_H
