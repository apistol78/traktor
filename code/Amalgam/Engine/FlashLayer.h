#ifndef traktor_amalgam_FlashLayer_H
#define traktor_amalgam_FlashLayer_H

#include "Amalgam/Engine/Layer.h"
#include "Core/Math/Vector2.h"
#include "Flash/Action/Avm1/Classes/As_flash_external_ExternalInterface.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Script/Any.h"
#include "Spray/Feedback/IFeedbackListener.h"

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

class IEnvironment;

	}

	namespace drawing
	{

class Image;

	}

	namespace flash
	{

class AccDisplayRenderer;
class ActionObject;
class FlashMovie;
class FlashMoviePlayer;
class ISoundRenderer;

	}

	namespace render
	{

class RenderTargetSet;		

	}

	namespace world
	{

class PostProcess;
class PostProcessSettings;

	}

	namespace amalgam
	{

/*! \brief Stage Flash layer.
 * \ingroup Amalgam
 */
class T_DLLCLASS FlashLayer
:	public Layer
,	public flash::IExternalCall
,	public spray::IFeedbackListener
{
	T_RTTI_CLASS;

public:
	FlashLayer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		amalgam::IEnvironment* environment,
		const resource::Proxy< flash::FlashMovie >& movie,
		const std::map< std::wstring, resource::Proxy< flash::FlashMovie > >& externalMovies,
		const resource::Proxy< world::PostProcessSettings >& postProcessSettings,
		bool clearBackground,
		bool enableSound
	);

	virtual ~FlashLayer();

	void destroy();

	virtual void transition(Layer* fromLayer);

	virtual void prepare();

	virtual void update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info);

	virtual void build(const amalgam::IUpdateInfo& info, uint32_t frame);

	virtual void render(render::EyeType eye, uint32_t frame);

	virtual void flush();

	virtual void preReconfigured();

	virtual void postReconfigured();

	virtual void suspend();

	virtual void resume();

	/*! \brief Get reference to current Flash movie player.
	 * 
	 * \return Flash movie player object.
	 */
	flash::FlashMoviePlayer* getMoviePlayer();

	/*! \brief Get ActionScript "_global" object.
	 *
	 * \return ActionScript "_global" object.
	 */
	flash::ActionObject* getGlobal();

	/*! \brief Get ActionScript "_root" object.
	 *
	 * \return ActionScript "_root" object.
	 */
	flash::ActionObject* getRoot();

	/*! \brief Create empty ActionScript object.
	 *
	 * \return ActionScript object.
	 */
	Ref< flash::ActionObject > createObject() const;

	/*! \brief Create ActionScript object.
	 *
	 * First argument should contain the name
	 * of the class prototype.
	 *
	 * \param argc Argument count.
	 * \param argv Arguments.
	 * \return ActionScript object.
	 */
	Ref< flash::ActionObject > createObject(uint32_t argc, const script::Any* argv) const;

	/*! \brief Create Flash bitmap object from image.
	 *
	 * \param image Image object.
	 * \return Flash bitmap object.
	 */
	Ref< flash::ActionObject > createBitmap(drawing::Image* image) const;

	/*! \brief Invoke methods registered through Flash ExternalInterface protocol.
	 *
	 * \param methodName Name of registered method.
	 * \param argc Argument count.
	 * \param argv Arguments.
	 * \return Return value from call.
	 */
	script::Any externalCall(const std::string& methodName, uint32_t argc, const script::Any* argv);

	/*! \brief Get "safe" string which only contain glyphs which are valid with loaded fonts.
	 *
	 * \param text Input text with potential unprintable glyphs.
	 * \param empty String to return if no glyphs in input text was valid.
	 * \return String containing only printable glyphs.
	 */
	std::wstring getPrintableString(const std::wstring& text, const std::wstring& empty) const;

	/*! \brief Set if Flash should be rendererd.
	 *
	 * \param visible True if Flash should be rendered.
	 */
	void setVisible(bool visible) { m_visible = visible; }

	/*! \brief Check if Flash is being rendered.
	 *
	 * \return True if Flash is being rendered.
	 */
	bool isVisible() const { return m_visible; }

private:
	struct LastMouseState
	{
		int32_t button;
		int32_t wheel;

		LastMouseState()
		:	button(0)
		,	wheel(0)
		{
		}
	};

	Ref< amalgam::IEnvironment > m_environment;
	resource::Proxy< flash::FlashMovie > m_movie;
	std::map< std::wstring, resource::Proxy< flash::FlashMovie > > m_externalMovies;
	Ref< flash::FlashMoviePlayer > m_moviePlayer;
	Ref< flash::AccDisplayRenderer > m_displayRenderer;
	Ref< flash::ISoundRenderer > m_soundRenderer;
	Ref< render::RenderTargetSet > m_postTargetSet;
	resource::Proxy< world::PostProcessSettings > m_postProcessSettings;
	Ref< world::PostProcess > m_postProcess;
	bool m_clearBackground;
	bool m_enableSound;
	bool m_visible;
	Vector2 m_offset;
	float m_scale;
	LastMouseState m_lastMouse[8];
	int32_t m_lastMouseX;
	int32_t m_lastMouseY;

	void createMoviePlayer();

	virtual flash::ActionValue dispatchExternalCall(const std::string& methodName, int32_t argc, const flash::ActionValue* argv);

	virtual void feedbackValues(spray::FeedbackType type, const float* values, int32_t count);
};

	}
}

#endif	// traktor_amalgam_FlashLayer_H
