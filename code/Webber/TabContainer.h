#ifndef traktor_wbr_TabContainer_H
#define traktor_wbr_TabContainer_H

#include "Webber/Container.h"

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
	
class T_DLLCLASS TabContainer : public Container
{
	T_RTTI_CLASS;

public:
	virtual void build(html::Element* parent) const override;

	virtual void consume(int32_t senderId, int32_t action) override;
};
	
	}
}

#endif	// traktor_wbr_TabContainer_H
