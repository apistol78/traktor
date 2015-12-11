#ifndef traktor_flash_FlashCharacterInstance_H
#define traktor_flash_FlashCharacterInstance_H

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Matrix33.h"
#include "Flash/SwfTypes.h"
#include "Flash/Action/ActionObjectRelay.h"

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
class FlashDictionary;
class IActionVMImage;

/*! \brief Character instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashCharacterInstance : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	FlashCharacterInstance(
		ActionContext* context,
		const char* const prototype,
		FlashDictionary* dictionary,
		FlashCharacterInstance* parent
	);

	virtual ~FlashCharacterInstance();

	/*! \brief Get character instance count. */
	static int32_t getInstanceCount();
	
	/*! \brief Destroy instance. */
	void destroy();

	/*! \brief Get ActionScript execution context.
	 *
	 * \return ActionScript context.
	 */
	ActionContext* getContext() const;

	/*! \brief Get dictionary.
	 *
	 * \return Dictionary.
	 */
	FlashDictionary* getDictionary() const;

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

	/*! \brief Get unique cache key.
	 */
	int32_t getCacheTag() const { return m_tag; }

	/*! \brief Get instance target path.
	 *
	 * \return Path of instance.
	 */
	std::string getTarget() const;

	/*! \brief Set color transform.
	 *
	 * \param cxform Color transform.
	 */
	void setColorTransform(const SwfCxTransform& cxform);

	/*! \brief Get color transform.
	 *
	 * \return Color transform.
	 */
	const SwfCxTransform& getColorTransform() const { return m_cxform; }

	/*! \brief Set coordinate transformation.
	 *
	 * \param transform Transformation matrix.
	 */
	void setTransform(const Matrix33& transform);

	/*! \brief Get coordinate transformation.
	 *
	 * \return Transformation matrix.
	 */
	const Matrix33& getTransform() const { return m_transform; }

	/*! \brief Get coordinate transformation; concatenated through parents.
	 *
	 * \return Transformation matrix.
	 */
	Matrix33 getFullTransform() const;

	/*! \brief
	 */
	void setFilter(uint8_t filter);

	/*! \brief
	 */
	uint8_t getFilter() const { return m_filter; }

	/*! \brief
	 */
	void setFilterColor(const SwfColor& filterColor);

	/*! \brief
	 */
	const SwfColor& getFilterColor() const { return m_filterColor; }

	/*! \brief
	 */
	void setBlendMode(uint8_t blendMode);

	/*! \brief
	 */
	uint8_t getBlendMode() const { return m_blendMode; }

	/*! \brief
	 */
	void setVisible(bool visible);

	/*! \brief
	 */
	bool isVisible() const { return m_visible; }

	/*! \brief
	 */
	void setEnabled(bool enabled);

	/*! \brief
	 */
	bool isEnabled() const { return m_enabled; }

	/*! \brief Set event scripts.
	 *
	 * \param eventScripts Event scripts.
	 */
	void setEvents(const SmallMap< uint32_t, Ref< const IActionVMImage > >& eventScripts);

	/*! \brief Get event scripts.
	 *
	 * \return Event scripts.
	 */
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& getEvents() const { return m_eventScripts; }

	/*! \name Events */
	//@{

	virtual void preDispatchEvents();

	virtual void postDispatchEvents();

	virtual void eventInit();

	virtual void eventConstruct();

	virtual void eventLoad();

	virtual void eventFrame();

	virtual void eventKey(wchar_t unicode);

	virtual void eventKeyDown(int keyCode);

	virtual void eventKeyUp(int keyCode);

	virtual void eventMouseDown(int x, int y, int button);

	virtual void eventMouseUp(int x, int y, int button);

	virtual void eventMouseMove0(int x, int y, int button);

	virtual void eventMouseMove1(int x, int y, int button);

	virtual void eventSetFocus();

	virtual void eventKillFocus();

	//@}

	/*! \name Character interface */
	//@{

	/*! \brief Get character bounding box.
	 *
	 * \return Bounding box.
	 */
	virtual Aabb2 getBounds() const = 0;

	//@}

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE;

protected:
	void setParent(FlashCharacterInstance* parent);

	bool haveScriptEvent(uint32_t eventName);

	bool executeScriptEvent(uint32_t eventName, const ActionValue& arg);

	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

private:
	static int32_t ms_instanceCount;
	Ref< ActionContext > m_context;
	Ref< FlashDictionary > m_dictionary;
	FlashCharacterInstance* m_parent;
	std::string m_name;
	int32_t m_tag;
	bool m_visible;
	bool m_enabled;
	SwfCxTransform m_cxform;
	Matrix33 m_transform;
	uint8_t m_filter;
	SwfColor m_filterColor;
	uint8_t m_blendMode;
	SmallMap< uint32_t, Ref< const IActionVMImage > > m_eventScripts;
};

	}
}

#endif	// traktor_flash_FlashCharacterInstance_H
