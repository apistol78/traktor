#ifndef traktor_flash_FlashCharacterInstance_H
#define traktor_flash_FlashCharacterInstance_H

#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"
#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;
class ActionScript;

/*! \brief Character instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashCharacterInstance : public ActionObject
{
	T_RTTI_CLASS;

public:
	FlashCharacterInstance(ActionContext* context, const std::string& prototypeName, FlashCharacterInstance* parent);
	
	/*! \brief Destroy instance. */
	virtual void destroy();

	/*! \brief Get ActionScript execution context.
	 *
	 * \return ActionScript context.
	 */
	ActionContext* getContext() const;

	/*! \brief Get parent instance.
	 *
	 * \return Parent instance.
	 */
	FlashCharacterInstance* getParent() const;

	/*! \brief Set instance name.
	 *
	 * \param name New name of instance.
	 */
	void setName(const std::string& name);

	/*! \brief Get instance name.
	 *
	 * \return Name of instance, empty string if not named.
	 */
	const std::string& getName() const;

	/*! \brief Set color transform.
	 *
	 * \param cxform Color transform.
	 */
	void setColorTransform(const SwfCxTransform& cxform);

	/*! \brief Get color transform.
	 *
	 * \return Color transform.
	 */
	const SwfCxTransform& getColorTransform() const;

	/*! \brief Set coordinate transformation.
	 *
	 * \param transform Transformation matrix.
	 */
	void setTransform(const Matrix33& transform);

	/*! \brief Get coordinate transformation.
	 *
	 * \return Transformation matrix.
	 */
	const Matrix33& getTransform() const;

	/*! \brief Set event script.
	 *
	 * \param eventMask Event mask.
	 * \param eventScript Event script.
	 */
	void setEvent(uint32_t eventMask, ActionScript* eventScript);

	/*! \brief Set event scripts.
	 *
	 * \param eventScripts Event scripts.
	 */
	void setEvents(const std::map< uint32_t, Ref< ActionScript > >& eventScripts);

	/*! \brief Get event scripts.
	 *
	 * \return Event scripts.
	 */
	const std::map< uint32_t, Ref< ActionScript > >& getEvents() const;

	/*! \name Events */
	//@{

	virtual void eventInit();

	virtual void eventLoad();

	virtual void eventFrame();

	virtual void eventKeyDown(int keyCode);

	virtual void eventKeyUp(int keyCode);

	virtual void eventMouseDown(int x, int y, int button);

	virtual void eventMouseUp(int x, int y, int button);

	virtual void eventMouseMove(int x, int y, int button);

	//@}

	/*! \name Character interface */
	//@{

	/*! \brief Get character bounding box.
	 *
	 * \return Bounding box.
	 */
	virtual SwfRect getBounds() const = 0;

	//@}

private:
	Ref< ActionContext > m_context;
	FlashCharacterInstance* m_parent;
	std::string m_name;
	SwfCxTransform m_cxform;
	Matrix33 m_transform;
	std::map< uint32_t, Ref< ActionScript > > m_eventScripts;
};

	}
}

#endif	// traktor_flash_FlashCharacterInstance_H
