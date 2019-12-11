#pragma once

#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

class Stage;
class UpdateControl;
class UpdateInfo;

/*! Abstract stage layer.
 * \ingroup Runtime
 */
class T_DLLCLASS Layer : public Object
{
	T_RTTI_CLASS;

public:
	Layer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition
	);

	virtual ~Layer();

	virtual void destroy();

	/*! Called during transition between stages.
	 *
	 * Transition is called for layers which have the
	 * same type and matching names.
	 *
	 * \param fromLayer Previous stage's layer.
	 */
	virtual void transition(Layer* fromLayer) = 0;

	/*! Prepare layer for update.
	 *
	 * \param info Engine update information.
	 */
	virtual void prepare(const UpdateInfo& info) = 0;

	/*! Update layer logic.
	 *
	 * info Engine update information.
	 */
	virtual void update(const UpdateInfo& info) = 0;

	/*! Build renderable context.
	 *
	 * \param info Engine update information.
	 * \param frame Render frame.
	 */
	virtual void build(const UpdateInfo& info, uint32_t frame) = 0;

	/*! Render previously build context.
	 *
	 * This is usually called from another thread than
	 * the "build" method so special care must be
	 * taken to ensure integrity of data.
	 *
	 * \param frame Render frame.
	 */
	virtual void render(uint32_t frame) = 0;

	/*! Flush anything pending; such as render contexts etc.
	 *
	 * Flush is called prior to state changes, configuration changes
	 * or application runtime state.
	 */
	virtual void flush() = 0;

	/*! Called before application is to be reconfigured. */
	virtual void preReconfigured() = 0;

	/*! Called after configuration has been applied. */
	virtual void postReconfigured() = 0;

	/*! Called when application is to be suspended. */
	virtual void suspend() = 0;

	/*! Called after application has resumed executing. */
	virtual void resume() = 0;

	/*! Get owner stage object.
	 *
	 * \return Stage object.
	 */
	Stage* getStage() const { return m_stage; }

	/*! Get name of this layer.
	 *
	 * \return This layer's name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! Return flag if transition of this layer is permitted.
	 *
	 * \return True if transition is permitted.
	 */
	bool isTransitionPermitted() const { return m_permitTransition; }

private:
	Stage* m_stage;
	std::wstring m_name;
	bool m_permitTransition;
};

	}
}

