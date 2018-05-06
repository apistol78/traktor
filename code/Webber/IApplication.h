#ifndef traktor_wbr_IApplication_H
#define traktor_wbr_IApplication_H

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEBBER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace html
	{
	
class Element;

	}

	namespace wbr
	{

class Widget;

class T_DLLCLASS IApplication : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create() = 0;

	virtual void destroy() = 0;

	virtual Widget* getRootWidget() const = 0;

	virtual Ref< IStream > resolve(const std::wstring& uri) const = 0;
};

	}
}

#endif	// traktor_wbr_IApplication_H
