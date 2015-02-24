#ifndef traktor_input_IfInclusiveExclusive_H
#define traktor_input_IfInclusiveExclusive_H

#include <list>
#include "Input/Binding/IInputFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! \brief Input include/exlusive filter.
 * \ingroup Input
 */
class T_DLLCLASS IfInclusiveExclusive : public IInputFilter
{
	T_RTTI_CLASS;
	
public:
	enum Priority
	{
		PrInclusive,
		PrExclusive
	};

	virtual Ref< Instance > createInstance() const;
	
	virtual void evaluate(Instance* instance, InputValueSet& valueSet) const;
	
	virtual void serialize(ISerializer& s);
	
private:
	Priority m_priority;
	std::list< std::wstring > m_inclusive;
	std::list< std::wstring > m_exclusive;
};
	
	}
}

#endif	// traktor_input_IfInclusiveExclusive_H
