#ifndef traktor_ui_SmartHandle_H
#define traktor_ui_SmartHandle_H

namespace traktor
{
	namespace ui
	{

struct GdiDeleteObjectPolicy
{
	static void deleteObject(HGDIOBJ h) { ::DeleteObject(h); }
};

template < typename HandleType, typename DeletePolicy >
class SmartHandle
{
public:
	SmartHandle() :
		m_h(0)
	{
	}

	SmartHandle(HandleType h) :
		m_h(h)
	{
	}

	~SmartHandle()
	{
		if (m_h)
			DeletePolicy::deleteObject(m_h);
	}

	SmartHandle& operator = (HandleType h)
	{
		if (m_h)
			DeletePolicy::deleteObject(m_h);
		m_h = h;
		return *this;
	}

	SmartHandle& operator = (SmartHandle& sh)
	{
		if (m_h)
			DeletePolicy::deleteObject(m_h);
		m_h = sh.m_h;
		sh.m_h = NULL;
		return *this;
	}

	HandleType getHandle() const
	{
		return m_h;
	}

	operator HandleType ()
	{
		return m_h;
	}

private:
	HandleType m_h;
};

typedef SmartHandle< HFONT,	GdiDeleteObjectPolicy >	SmartFont;
typedef SmartHandle< HBRUSH, GdiDeleteObjectPolicy > SmartBrush;
typedef SmartHandle< HPEN,	GdiDeleteObjectPolicy >	SmartPen;

	}
}

#endif	// traktor_ui_SmartHandle_H
