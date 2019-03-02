#pragma once

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

	virtual std::wstring getDescription() const override final;

	virtual void prepare(float T, float dT) override final;

	virtual float read(float T, float dT) override final;

private:
	Ref< const ControlInputSourceData > m_data;
	Ref< DeviceControlManager > m_deviceControlManager;
};

	}
}

