#ifndef traktor_input_InReadValue_H
#define traktor_input_InReadValue_H

#include "Input/Binding/IInputNode.h"

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

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InReadValue : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InReadValue();
	
	InReadValue(const std::wstring& valueId);
	
	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;	

	virtual void serialize(ISerializer& s);
	
private:
	std::wstring m_valueId;
};

	}
}

#endif	// traktor_input_InReadValue_H
