#ifndef traktor_input_ControlInputSource_H
#define traktor_input_ControlInputSource_H

#include "Core/RefArray.h"
#include "Input/Binding/IInputSource.h"

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

class DeviceControlManager;
class ControlInputSourceData;

/*! \brief Control input source.
 * \ingroup Input
 */
class T_DLLCLASS ControlInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	ControlInputSource(const ControlInputSourceData* data, DeviceControlManager* deviceControlManager);
	
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual void prepare(float T, float dT) T_OVERRIDE T_FINAL;

	virtual float read(float T, float dT) T_OVERRIDE T_FINAL;
	
private:
	Ref< const ControlInputSourceData > m_data;
	Ref< DeviceControlManager > m_deviceControlManager;
};

	}
}

#endif	// traktor_input_ControlInputSource_H
