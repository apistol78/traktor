#ifndef traktor_input_CombinedInputSourceData_H
#define traktor_input_CombinedInputSourceData_H

#include "Input/InputTypes.h"
#include "Input/Binding/IInputSourceData.h"

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
	
class T_DLLCLASS CombinedInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	CombinedInputSourceData();
	
	CombinedInputSourceData(
		IInputSourceData* source1,
		IInputSourceData* source2
	);

	void setSource1(IInputSourceData* source1);

	IInputSourceData* getSource1() const;

	void setSource2(IInputSourceData* source2);

	IInputSourceData* getSource2() const;
	
	virtual Ref< IInputSource > createInstance() const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< IInputSourceData > m_source1;
	Ref< IInputSourceData > m_source2;
};

	}
}

#endif	// traktor_input_CombinedInputSourceData_H
