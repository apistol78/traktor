#pragma once

#include "Runtime/Engine/Layer.h"
#include "Core/Class/Any.h"
#include "Core/Math/Vector2.h"
#include "Spark/Action/Common/Classes/As_flash_external_ExternalInterface.h"
#include "Resource/Id.h"
#include "Resource/Proxy.h"
#include "Spray/Feedback/IFeedbackListener.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

class IEnvironment;

	}

	namespace render
	{

class ImageProcess;
class ImageProcessData;
class IRenderTargetSet;

	}

	namespace spark
	{

class AccDisplayRenderer;
class ActionObject;
class Movie;
class MoviePlayer;
class MovieRenderer;
class SpriteInstance;
class ISoundRenderer;

/*! Stage Spark layer.
 * \ingroup Spark
 */
class T_DLLCLASS SparkLayer
:	public runtime::Layer
,	public IExternalCall
,	public spray::IFeedbackListener
{
	T_RTTI_CLASS;

public:
	SparkLayer(
		runtime::Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		runtime::IEnvironment* environment,
		const resource::Proxy< Movie >& movie,
		const std::map< std::wstring, resource::Proxy< Movie > >& externalMovies,
		const resource::Proxy< render::ImageProcessData >& imageProcessSettings,
		bool clearBackground,
		bool enableShapeCache,
		bool enableDirtyRegions,
		bool enableSound,
		uint32_t contextSize
	);

	virtual void destroy() override;

	virtual void transition(Layer* fromLayer) override final;

	virtual void prepare(const runtime::UpdateInfo& info) override final;

	virtual void update(const runtime::UpdateInfo& info) override final;

	virtual void build(const runtime::UpdateInfo& info, uint32_t frame) override final;

	virtual void render(uint32_t frame) override final;

	virtual void flush() override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

	/*! Get reference to current Spark movie player.
	 *
	 * \return Spark movie player object.
	 */
	MoviePlayer* getMoviePlayer();

	/*! Get ActionScript context.
	 *
	 * \return ActionScript context.
	 */
	ActionContext* getContext();

	/*! Get "_root" sprite.
	 *
	 * \return "_root" sprite.
	 */
	SpriteInstance* getRoot();

	/*! Get external movie.
	 *
	 * \param id Name of external movie.
	 * \return External movie, null if doesn't exist.
	 */
	Movie* getExternal(const std::wstring& id) const;

	/*! Invoke methods registered through Spark ExternalInterface protocol.
	 *
	 * \param methodName Name of registered method.
	 * \param argc Argument count.
	 * \param argv Arguments.
	 * \return Return value from call.
	 */
	Any externalCall(const std::string& methodName, uint32_t argc, const Any* argv);

	/*! Get "safe" string which only contain glyphs which are valid with loaded fonts.
	 *
	 * \param text Input text with potential unprintable glyphs.
	 * \param empty String to return if no glyphs in input text was valid.
	 * \return String containing only printable glyphs.
	 */
	std::wstring getPrintableString(const std::wstring& text, const std::wstring& empty) const;

	/*! Set if Spark should be rendererd.
	 *
	 * \param visible True if Spark should be rendered.
	 */
	void setVisible(bool visible) { m_visible = visible; }

	/*! Check if Spark is being rendered.
	 *
	 * \return True if Spark is being rendered.
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

	Ref< runtime::IEnvironment > m_environment;
	resource::Proxy< Movie > m_movie;
	std::map< std::wstring, resource::Proxy< Movie > > m_externalMovies;
	Ref< MoviePlayer > m_moviePlayer;
	Ref< MovieRenderer > m_movieRenderer;
	Ref< AccDisplayRenderer > m_displayRenderer;
	Ref< ISoundRenderer > m_soundRenderer;
	Ref< render::IRenderTargetSet > m_imageTargetSet;
	resource::Proxy< render::ImageProcessData > m_imageProcessSettings;
	Ref< render::ImageProcess > m_imageProcess;
	bool m_clearBackground;
	bool m_enableShapeCache;
	bool m_enableDirtyRegions;
	bool m_enableSound;
	uint32_t m_contextSize;
	bool m_visible;
	Vector2 m_offset;
	float m_scale;
	bool m_lastUpValue;
	bool m_lastDownValue;
	bool m_lastConfirmValue;
	bool m_lastEscapeValue;
	LastMouseState m_lastMouse[8];
	int32_t m_lastMouseX;
	int32_t m_lastMouseY;

	void createMoviePlayer();

	virtual ActionValue dispatchExternalCall(const std::string& methodName, int32_t argc, const ActionValue* argv) override final;

	virtual void feedbackValues(spray::FeedbackType type, const float* values, int32_t count) override final;
};

	}
}

