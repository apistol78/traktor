#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Matrix33.h"
#include "Spark/ColorTransform.h"
#include "Spark/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Context;
class Dictionary;

/*! Character instance.
 * \ingroup Spark
 */
#if defined (_MSC_VER)
#	pragma warning( disable:4324 )
#endif
class T_DLLCLASS CharacterInstance : public Object
{
	T_RTTI_CLASS;

public:
	explicit CharacterInstance(
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent
	);

	virtual ~CharacterInstance();

	/*! Get character instance count. */
	static int32_t getInstanceCount();

	/*! Destroy instance. */
	virtual void destroy();

	/*! Get context.
	 *
	 * \return Context.
	 */
	Context* getContext() const { return m_context; }

	/*! Get dictionary.
	 *
	 * \return Dictionary.
	 */
	Dictionary* getDictionary() const { return m_dictionary; }

	/*! Set parent instance.
	 */
	void setParent(CharacterInstance* parent);

	/*! Get parent instance.
	 *
	 * \return Parent instance.
	 */
	CharacterInstance* getParent() const { return m_parent; }

	/*! Set instance name.
	 *
	 * \param name New name of instance.
	 */
	void setName(const std::string& name);

	/*! Get instance name.
	 *
	 * \return Name of instance, empty string if not named.
	 */
	const std::string& getName() const;

	/*! Set cache object.
	 */
	void setCacheObject(IRefCount* cacheObject);

	/*! Clear cache object.
	 */
	virtual void clearCacheObject();

	/*! Set cache object.
	 */
	IRefCount* getCacheObject() { return m_cacheObject; }

	/*! Set user defined object.
	 */
	void setUserObject(IRefCount* userObject);

	/*! Get user defined object.
	 */
	IRefCount* getUserObject() { return m_userObject; }

	/*! Get instance target path.
	 *
	 * \return Path of instance.
	 */
	std::string getTarget() const;

	/*! Set color transform.
	 *
	 * \param cxform Color transform.
	 */
	void setColorTransform(const ColorTransform& cxform);

	/*! Get color transform.
	 *
	 * \return Color transform.
	 */
	const ColorTransform& getColorTransform() const { return m_cxform; }

	/*! Get color transform; concatenated through parents.
	 *
	 * \return Color transform.
	 */
	ColorTransform getFullColorTransform() const;

	/*! Set alpha value in color transform.
	 */
	void setAlpha(float alpha);

	/*! Get alpha value from color transform.
	 */
	float getAlpha() const;

	/*! Set coordinate transformation.
	 *
	 * \param transform Transformation matrix.
	 */
	void setTransform(const Matrix33& transform);

	/*! Get coordinate transformation.
	 *
	 * \return Transformation matrix.
	 */
	const Matrix33& getTransform() const { return m_transform; }

	/*! Get coordinate transformation; concatenated through parents.
	 *
	 * \return Transformation matrix.
	 */
	Matrix33 getFullTransform() const;

	/*! Transform between characters.
	 */
	Vector2 transformInto(const CharacterInstance* other, const Vector2& pnt) const;

	/*!
	 */
	void setFilter(uint8_t filter);

	/*!
	 */
	uint8_t getFilter() const { return m_filter; }

	/*!
	 */
	void setFilterColor(const Color4f& filterColor);

	/*!
	 */
	const Color4f& getFilterColor() const { return m_filterColor; }

	/*!
	 */
	void setBlendMode(uint8_t blendMode);

	/*!
	 */
	uint8_t getBlendMode() const { return m_blendMode; }

	/*!
	 */
	void setVisible(bool visible);

	/*!
	 */
	bool isVisible() const { return m_visible; }

	/*!
	 */
	void setEnabled(bool enabled);

	/*!
	 */
	bool isEnabled() const { return m_enabled; }

	/*!
	 */
	void setFocus();

	/*!
	 */
	bool haveFocus() const;

	/*!
	 */
	void setWireOutline(bool wireOutline);

	/*!
	 */
	bool getWireOutline() const { return m_wireOutline; }

	/*! \name Events */
	//@{

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

	/*! Get character bounding box.
	 *
	 * \return Bounding box.
	 */
	virtual Aabb2 getBounds() const = 0;

	//@}

	/*! \group Events */
	//@{

	Event* getEventSetFocus() { return &m_eventSetFocus; }

	Event* getEventKillFocus() { return &m_eventKillFocus; }

	//@}

private:
	static std::atomic< int32_t > ms_instanceCount;

	std::string m_name;
	Matrix33 m_transform;
	ColorTransform m_cxform;
	Ref< Context > m_context;
	Ref< Dictionary > m_dictionary;
	CharacterInstance* m_parent;
	Ref< IRefCount > m_cacheObject;
	Ref< IRefCount > m_userObject;
	Color4f m_filterColor;
	uint8_t m_filter;
	uint8_t m_blendMode;
	bool m_visible;
	bool m_enabled;
	bool m_wireOutline;
	Event m_eventSetFocus;
	Event m_eventKillFocus;
};
#if defined (_MSC_VER)
#	pragma warning( default:4324 )
#endif

	}
}

