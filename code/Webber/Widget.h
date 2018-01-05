#ifndef traktor_wbr_Widget_H
#define traktor_wbr_Widget_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;

	namespace html
	{
	
class Element;

	}

	namespace wbr
	{
	
class T_DLLCLASS Widget : public Object
{
	T_RTTI_CLASS;

public:
	Widget();

	int32_t getId() const { return m_id; }

	virtual void build(html::Element* parent) const = 0;

	virtual void consume(int32_t senderId, int32_t action) = 0;

private:
	int32_t m_id;
};
	
	}
}

#endif	// traktor_wbr_Widget_H
