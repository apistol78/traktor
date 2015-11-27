#ifndef traktor_input_CombinedInputSourceData_H
#define traktor_input_CombinedInputSourceData_H

#include "Input/InputTypes.h"
#include "Input/Binding/CombinedInputSource.h"
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
	
/*! \brief Boolean combination of multiple input sources.
 * \ingroup Input
 */
class T_DLLCLASS CombinedInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	CombinedInputSourceData();

	CombinedInputSourceData(CombinedInputSource::CombineMode mode);
	
	CombinedInputSourceData(const RefArray< IInputSourceData >& sources, CombinedInputSource::CombineMode mode);

	void addSource(IInputSourceData* source);

	const RefArray< IInputSourceData >& getSources() const;
	
	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	RefArray< IInputSourceData > m_sources;
	CombinedInputSource::CombineMode m_mode;
};

	}
}

#endif	// traktor_input_CombinedInputSourceData_H
