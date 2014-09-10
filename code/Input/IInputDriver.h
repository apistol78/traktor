#ifndef traktor_input_IInputDriver_H
#define traktor_input_IInputDriver_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct SystemWindow;

	namespace input
	{

class IInputDevice;

/*! \brief Input driver base.
 * \ingroup Input
 */
class T_DLLCLASS IInputDriver : public Object
{
	T_RTTI_CLASS;

public:
	enum UpdateResult
	{
		UrOk = 0,
		UrDevicesChanged = 1,
		UrFailure = -1
	};

	/*! \brief Create input driver.
	 *
	 * \param nativeHandle Opaque pointer to system specific object.
	 * \param systemWindow Description of application's system window.
	 * \param inputCategories Input device categories.
	 */
	virtual bool create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories) = 0;
	
	/*! \brief Get number of devices implemented by the driver.
	 *
	 * \note Users should not call this method.
	 *
	 * \return Number of devices.
	 */
	virtual int getDeviceCount() = 0;

	/*! \brief Get device from ordinal.
	 *
	 * \note Users should not call this method.
	 *
	 * \param index Device ordinal; must be in 0 to getDeviceCount()-1 range.
	 * \return Device implementation.
	 */
	virtual Ref< IInputDevice > getDevice(int index) = 0;
	
	/*! \brief Update driver.
	 *
	 * During driver update devices might be connected or disconnected
	 * thus the driver implementation must return UrDevicesChanged in
	 * such case to let the input system update it's mapping.
	 *
	 * \note Users should not call this method.
	 *
	 * \return Update result.
	 */
	virtual UpdateResult update() = 0;
};

	}
}

#endif	// traktor_input_IInputDriver_H
