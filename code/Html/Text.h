#pragma once

#include "Html/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace html
	{

/*! HTML DOM Text.
 * \ingroup HTML
 */
class T_DLLCLASS Text : public Node
{
	T_RTTI_CLASS;

public:
	explicit Text(const std::wstring& text);

	virtual std::wstring getName() const override final;

	virtual std::wstring getValue() const override final;

	virtual void toString(OutputStream& os) const override final;

private:
	std::wstring m_text;
};

	}
}

