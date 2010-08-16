#ifndef traktor_input_InCombine_H
#define traktor_input_InCombine_H

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

class T_DLLCLASS InCombine : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InCombine();

	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;
	
	virtual bool serialize(ISerializer& s);
	
private:
	struct Entry
	{
		Ref< IInputNode > source;
		float mul;
		float add;

		bool serialize(ISerializer& s);
	};

	std::vector< Entry > m_entries;
};

	}
}

#endif	// traktor_input_InCombine_H
