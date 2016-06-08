#ifndef traktor_amalgam_Layer_H
#define traktor_amalgam_Layer_H

#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class Stage;
class UpdateControl;
class UpdateInfo;

/*! \brief Abstract stage layer.
 * \ingroup Amalgam
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

	/*! \brief Called during transition between stages.
	 *
	 * Transition is called for layers which have the
	 * same type and matching names.
	 *
	 * \param fromLayer Previous stage's layer.
	 */
	virtual void transition(Layer* fromLayer) = 0;

	/*! \brief Prepare layer for update.
	 *
	 * \param info Engine update information.
	 */
	virtual void prepare(const UpdateInfo& info) = 0;

	/*! \brief Update layer logic.
	 *
	 * info Engine update information.
	 */
	virtual void update(const UpdateInfo& info) = 0;

	/*! \brief Build renderable context.
	 *
	 * \param info Engine update information.
	 * \param frame Render frame.
	 */
	virtual void build(const UpdateInfo& info, uint32_t frame) = 0;

	/*! \brief Render previously build context.
	 *
	 * This is usually called from another thread than
	 * the "build" method so special care must be
	 * taken to ensure integrity of data.
	 *
	 * \param eye Render eye.
	 * \param frame Render frame.
	 */
	virtual void render(render::EyeType eye, uint32_t frame) = 0;

	/*! \brief Flush anything pending; such as render contexts etc.
	 *
	 * Flush is called prior to state changes, configuration changes
	 * or application runtime state.
	 */
	virtual void flush() = 0;

	/*! \brief Called before application is to be reconfigured. */
	virtual void preReconfigured() = 0;

	/*! \brief Called after configuration has been applied. */
	virtual void postReconfigured() = 0;

	/*! \brief Called when application is to be suspended. */
	virtual void suspend() = 0;

	/*! \brief Called after application has resumed executing. */
	virtual void resume() = 0;

	/*! \brief Get owner stage object.
	 *
	 * \return Stage object.
	 */
	Stage* getStage() const { return m_stage; }

	/*! \brief Get name of this layer.
	 *
	 * \return This layer's name.
	 */
	const std::wstring& getName() const { return m_name; }

	/*! \brief Return flag if transition of this layer is permitted.
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

#endif	// traktor_amalgam_Layer_H
