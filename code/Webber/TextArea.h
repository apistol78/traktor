#ifndef traktor_wbr_TextArea_H
#define traktor_wbr_TextArea_H

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
	
class T_DLLCLASS TextArea : public Widget
{
	T_RTTI_CLASS;

public:
	TextArea();

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_wbr_TextArea_H
