#ifndef traktor_amalgam_StateManager_H
#define traktor_amalgam_StateManager_H

#include "Core/Thread/Semaphore.h"
#include "Amalgam/IStateManager.h"

namespace traktor
{
	namespace amalgam
	{

/*! \brief State manager. */
class StateManager : public IStateManager
{
	T_RTTI_CLASS;

public:
	/*! \brief Destroy state manager. */
	void destroy();

	/*! \brief Begin update transition.
	 *
	 * \return True if transition needs to be performed.
	 */
	bool beginTransition();

	/*! \brief Perform transition.
	 *
	 * \return True if state transition completed.
	 */
	bool performTransition();

	/*! \brief Enter state transition.
	 *
	 * \param state New state.
	 */
	virtual void enter(IState* state);

	/*! \brief Get current state.
	 *
	 * \return Current state.
	 */
	IState* getCurrent() { return m_current; }

	/*! \brief Get next state.
	 *
	 * \return Next state.
	 */
	IState* getNext() { return m_next; }

private:
	Semaphore m_lock;
	Ref< IState > m_current;
	Ref< IState > m_next;
};

	}
}

#endif	// traktor_amalgam_StateManager_H
