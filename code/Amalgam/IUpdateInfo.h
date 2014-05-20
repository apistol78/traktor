#ifndef traktor_amalgam_IUpdateInfo_H
#define traktor_amalgam_IUpdateInfo_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class FrameProfiler;

/*! \brief Update information.
 * \ingroup Amalgam
 *
 * Update information provide time measurements
 * to user applications.
 */
class T_DLLCLASS IUpdateInfo : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Get total application time.
	 *
	 * \return Time in seconds.
	 */
	virtual float getTotalTime() const = 0;

	/*! \brief Get real time in current state.
	 *
	 * \return Time in seconds.
	 */
	virtual float getStateTime() const = 0;

	/*! \brief Get simulation time in current state.
	 *
	 * \note Simulation time are updated in discreet
	 * time steps thus might not be same as state time.
	 * Can be used to derive frame interpolation offset
	 * as \"offset = simulation time - state time\"
	 *
	 * \return Time in seconds.
	 */
	virtual float getSimulationTime() const = 0;

	/*! \brief Get simulation delta time.
	 *
	 * Simulation delta time is a fixed value
	 * defined at the beginning of the application.
	 *
	 * \return Delta time in seconds.
	 */
	virtual float getSimulationDeltaTime() const = 0;

	/*! \brief Get simulation frequency.
	 *
	 * \return Frequency in hertz.
	 */
	virtual int32_t getSimulationFrequency() const = 0;

	/*! \brief Get frame delta time.
	 *
	 * Frame delta time varies depending on current
	 * frame-rate.
	 *
	 * \return Delta time in seconds.
	 */
	virtual float getFrameDeltaTime() const = 0;

	/*! \brief Get frame count.
	 *
	 * \return Frame count since beginning of application.
	 */
	virtual uint64_t getFrame() const = 0;

	/*! \brief Get interval fraction.
	 *
	 * I = 1 - (Tsim - Tstate) / dTsim
	 *
	 * \return Interval fraction.
	 */
	virtual float getInterval() const = 0;

	/*! \brief If system is detected as being too slow.
	 *
	 * \return True if system incapable of sustaining a stable frame rate.
	 */
	virtual bool isRunningSlow() const = 0;

	/*! \brief Access the frame profiler.
	 */
	virtual FrameProfiler* getProfiler() const = 0;
};

	}
}

#endif	// traktor_amalgam_IUpdateInfo_H
