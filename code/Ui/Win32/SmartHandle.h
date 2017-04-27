/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_SmartHandle_H
#define traktor_ui_SmartHandle_H

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
struct GdiDeleteObjectPolicy
{
	static void deleteObject(HGDIOBJ h) { ::DeleteObject(h); }
};

/*! \brief
 * \ingroup UIW32
 */
template < typename HandleType, typename DeletePolicy >
class SmartHandle
{
public:
	SmartHandle()
	:	m_object(0)
	{
	}

	SmartHandle(HandleType h)
	:	m_object(h)
	{
	}

	~SmartHandle()
	{
		if (m_object)
			DeletePolicy::deleteObject(m_object);
	}

	SmartHandle& operator = (HandleType h)
	{
		if (m_object)
			DeletePolicy::deleteObject(m_object);
		m_object = h;
		return *this;
	}

	SmartHandle& operator = (SmartHandle& sh)
	{
		if (m_object)
			DeletePolicy::deleteObject(m_object);
		m_object = sh.m_object;
		sh.m_object = NULL;
		return *this;
	}

	HandleType getHandle() const
	{
		return m_object;
	}

	operator HandleType ()
	{
		return m_object;
	}

private:
	HandleType m_object;
};

typedef SmartHandle< HFONT,	GdiDeleteObjectPolicy >	SmartFont;
typedef SmartHandle< HBRUSH, GdiDeleteObjectPolicy > SmartBrush;
typedef SmartHandle< HPEN,	GdiDeleteObjectPolicy >	SmartPen;

	}
}

#endif	// traktor_ui_SmartHandle_H
