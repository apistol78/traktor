/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_CharacterInstance_H
#define traktor_flash_CharacterInstance_H

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"
#include "Flash/ColorTransform.h"
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
class Dictionary;
class IActionVMImage;

/*! \brief Character instance.
 * \ingroup Flash
 */
#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif
class T_DLLCLASS CharacterInstance : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	CharacterInstance(
		ActionContext* context,
		const char* const prototype,
		Dictionary* dictionary,
		CharacterInstance* parent
	);

	virtual ~CharacterInstance();

	/*! \brief Get character instance count. */
	static int32_t getInstanceCount();
	
	/*! \brief Destroy instance. */
	virtual void destroy();

	/*! \brief Get ActionScript execution context.
	 *
	 * \return ActionScript context.
	 */
	ActionContext* getContext() const { return m_context; }

	/*! \brief Get dictionary.
	 *
	 * \return Dictionary.
	 */
	Dictionary* getDictionary() const { return m_dictionary; }

	/*! \brief Set parent instance.
	 */
	void setParent(CharacterInstance* parent);

	/*! \brief Get parent instance.
	 *
	 * \return Parent instance.
	 */
	CharacterInstance* getParent() const { return m_parent; }

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

	/*! \brief Set cache object.
	 */
	void setCacheObject(IRefCount* cacheObject);

	/*! \brief Clear cache object.
	 */
	virtual void clearCacheObject();

	/*! \brief Set cache object.
	 */
	IRefCount* getCacheObject() { return m_cacheObject; }

	/*! \brief Set user defined object.
	 */
	void setUserObject(IRefCount* userObject);

	/*! \brief Get user defined object.
	 */
	IRefCount* getUserObject() { return m_userObject; }

	/*! \brief Get instance target path.
	 *
	 * \return Path of instance.
	 */
	std::string getTarget() const;

	/*! \brief Set color transform.
	 *
	 * \param cxform Color transform.
	 */
	void setColorTransform(const ColorTransform& cxform);

	/*! \brief Get color transform.
	 *
	 * \return Color transform.
	 */
	const ColorTransform& getColorTransform() const { return m_cxform; }

	/*! \brief Get color transform; concatenated through parents.
	 *
	 * \return Color transform.
	 */
	ColorTransform getFullColorTransform() const;

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

	/*! \brief Transform between characters.
	 */
	Vector2 transformInto(const CharacterInstance* other, const Vector2& pnt) const;

	/*! \brief
	 */
	void setFilter(uint8_t filter);

	/*! \brief
	 */
	uint8_t getFilter() const { return m_filter; }

	/*! \brief
	 */
	void setFilterColor(const Color4f& filterColor);

	/*! \brief
	 */
	const Color4f& getFilterColor() const { return m_filterColor; }

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

	/*! \brief
	 */
	void setFocus();

	/*! \brief
	 */
	bool haveFocus() const;

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

	virtual void eventInit();

	virtual void eventConstruct();

	virtual void eventLoad();

	virtual void eventFrame();

	virtual void eventKey(wchar_t unicode);

	virtual void eventKeyDown(int keyCode);

	virtual void eventKeyUp(int keyCode);

	virtual void eventMouseDown(int x, int y, int button);

	virtual void eventMouseUp(int x, int y, int button);

	virtual void eventMouseMove(int x, int y, int button);

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

	/*! \brief Check if character has associated script event.
	 */
	bool haveScriptEvent(uint32_t eventName);

	/*! \brief Execute script event associated with character.
	 */
	bool executeScriptEvent(uint32_t eventName, const ActionValue& arg);

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

private:
	static int32_t ms_instanceCount;

	std::string m_name;
	Matrix33 m_transform;
	ColorTransform m_cxform;
	Ref< ActionContext > m_context;
	Ref< Dictionary > m_dictionary;
	CharacterInstance* m_parent;
	Ref< IRefCount > m_cacheObject;
	Ref< IRefCount > m_userObject;
	SmallMap< uint32_t, Ref< const IActionVMImage > > m_eventScripts;
	Color4f m_filterColor;
	uint8_t m_filter;
	uint8_t m_blendMode;
	bool m_visible;
	bool m_enabled;
};
#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

#endif	// traktor_flash_CharacterInstance_H
