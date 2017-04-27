/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_html_Text_H
#define traktor_html_Text_H

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

/*! \brief HTML DOM Text.
 * \ingroup HTML
 */
class T_DLLCLASS Text : public Node
{
	T_RTTI_CLASS;

public:
	Text(const std::wstring& text);

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual std::wstring getValue() const T_OVERRIDE T_FINAL;

	virtual void writeHtml(IStream* stream) T_OVERRIDE T_FINAL;

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_html_Text_H
