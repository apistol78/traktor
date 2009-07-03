#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const Vector4 c_axis[] =
{
	Vector4(1.0f, 0.0f, 0.0f, 0.0f),
	Vector4(0.0f, 1.0f, 0.0f, 0.0f),
	Vector4(0.0f, 0.0f, 1.0f, 0.0f)
};

void getMouseDeltaAxises(
	const Matrix44& worldView,
	int& outMouseX, int& outMouseY, int& outMouseZ,
	float& outScaleX, float& outScaleY, float& outScaleZ
)
{
	const Vector4 axisX = worldView.axisX();
	const Vector4 axisY = worldView.axisY();
	const Vector4 axisZ = worldView.axisZ();

	if (abs(axisX.x()) > abs(axisY.x()))
	{
		if (abs(axisX.x()) > abs(axisZ.x()))
		{
			outMouseX = 0;
			outScaleX = axisX.x() >= 0.0f ? 1.0f : -1.0f;
		}
		else
		{
			outMouseX = 2;
			outScaleX = axisZ.x() >= 0.0f ? 1.0f : -1.0f;
		}
	}
	else if (abs(axisY.x()) > abs(axisZ.x()))
	{
		outMouseX = 1;
		outScaleX = axisY.x() >= 0.0f ? 1.0f : -1.0f;
	}
	else
	{
		outMouseX = 2;
		outScaleX = axisZ.x() >= 0.0f ? 1.0f : -1.0f;
	}

	if (abs(axisX.y()) > abs(axisY.y()))
	{
		if (abs(axisX.y()) > abs(axisZ.y()))
		{
			outMouseY = 0;
			outScaleY = axisX.y() >= 0.0f ? 1.0f : -1.0f;
		}
		else
		{
			outMouseY = 2;
			outScaleY = axisZ.y() >= 0.0f ? 1.0f : -1.0f;
		}
	}
	else if (abs(axisY.y()) > abs(axisZ.y()))
	{
		outMouseY = 1;
		outScaleY = axisY.y() >= 0.0f ? 1.0f : -1.0f;
	}
	else
	{
		outMouseY = 2;
		outScaleY = axisZ.y() >= 0.0f ? 1.0f : -1.0f;
	}

	const int c_cross[3][3] =
	{
		{ 0, 2, 1 },
		{ 2, 0, 0 },
		{ 1, 0, 0 }
	};

	outMouseZ = c_cross[outMouseX][outMouseY];

	switch (outMouseZ)
	{
	case 0:
		outScaleZ = axisX.z() >= 0.0f ? 1.0f : -1.0f;
		break;
	case 1:
		outScaleZ = axisY.z() >= 0.0f ? 1.0f : -1.0f;
		break;
	case 2:
		outScaleZ = axisZ.z() >= 0.0f ? 1.0f : -1.0f;
		break;
	}

	T_ASSERT (outMouseX != outMouseY);
	T_ASSERT (outMouseX != outMouseZ);
	T_ASSERT (outMouseY != outMouseZ);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TranslateModifier", TranslateModifier, Modifier)

void TranslateModifier::draw(
	SceneEditorContext* context,
	const Matrix44& worldTransform,
	render::PrimitiveRenderer* primitiveRenderer,
	bool active,
	int button
)
{
	const Scalar c_guideScale(0.1f);
	const Scalar c_guideMinLength(1.0f);

	Matrix44 world = Matrix44::identity();

	switch (context->getEditSpace())
	{
	case SceneEditorContext::EsWorld:
		world = translate(worldTransform.translation());
		break;
	case SceneEditorContext::EsObject:
		world = worldTransform;
		break;
	}

	Scalar cameraDistance = world.translation().length();
	Scalar guideLength = max(cameraDistance * c_guideScale, c_guideMinLength);
	Scalar arrowLength = guideLength * Scalar(1.0f / 8.0f);

	int mouseX, mouseY, mouseZ;
	float scaleX, scaleY, scaleZ;
	getMouseDeltaAxises(
		world * primitiveRenderer->getView(),
		mouseX, mouseY, mouseZ,
		scaleX, scaleY, scaleZ
	);

	Vector4 axisX = c_axis[mouseX] * Scalar(scaleX);
	Vector4 axisY = c_axis[mouseY] * Scalar(scaleY);
	Vector4 axisZ = c_axis[mouseZ] * Scalar(scaleZ);

	primitiveRenderer->pushWorld(world);

	uint32_t axisEnable = context->getAxisEnable();

	if (!active)
	{
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			axisX * guideLength,
			3.0f,
			Color(255, 0, 0, (axisEnable & SceneEditorContext::AeX) ? 255 : 64)
		);
		if (axisEnable & SceneEditorContext::AeX)
			primitiveRenderer->drawArrowHead(
				axisX * guideLength,
				axisX * (guideLength + arrowLength),
				0.5f,
				Color(255, 0, 0, 255)
			);

		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			axisY * guideLength,
			3.0f,
			Color(0, 255, 0, (axisEnable & SceneEditorContext::AeY) ? 255 : 64)
		);
		if (axisEnable & SceneEditorContext::AeY)
			primitiveRenderer->drawArrowHead(
				axisY * guideLength,
				axisY * (guideLength + arrowLength),
				0.5f,
				Color(0, 255, 0, 255)
			);

		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			axisZ * guideLength,
			3.0f,
			Color(0, 0, 255, (axisEnable & SceneEditorContext::AeZ) ? 255 : 64)
		);
		if (axisEnable & SceneEditorContext::AeZ)
			primitiveRenderer->drawArrowHead(
				axisZ * guideLength,
				axisZ * (guideLength + arrowLength),
				0.5f,
				Color(0, 0, 255, 255)
			);
	}
	else
	{
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, 0.0f, 1.0f),
			axisX * guideLength,
			3.0f,
			Color(255, 0, 0, (axisEnable & SceneEditorContext::AeX) ? 255 : 64)
		);
		if (axisEnable & SceneEditorContext::AeX)
			primitiveRenderer->drawArrowHead(
				axisX * guideLength,
				axisX * (guideLength + arrowLength),
				0.5f,
				Color(255, 0, 0, 255)
			);

		if (button == 0)
		{
			primitiveRenderer->drawLine(
				Vector4(0.0f, 0.0f, 0.0f, 1.0f),
				axisY * guideLength,
				3.0f,
				Color(0, 255, 0, (axisEnable & SceneEditorContext::AeY) ? 255 : 64)
			);
			if (axisEnable & SceneEditorContext::AeY)
				primitiveRenderer->drawArrowHead(
					axisY * guideLength,
					axisY * (guideLength + arrowLength),
					0.5f,
					Color(0, 255, 0, 255)
				);
		}
		else
		{
			primitiveRenderer->drawLine(
				Vector4(0.0f, 0.0f, 0.0f, 1.0f),
				axisZ * guideLength,
				3.0f,
				Color(0, 0, 255, (axisEnable & SceneEditorContext::AeZ) ? 255 : 64)
			);
			if (axisEnable & SceneEditorContext::AeZ)
				primitiveRenderer->drawArrowHead(
					axisZ * guideLength,
					axisZ * (guideLength + arrowLength),
					0.5f,
					Color(0, 0, 255, 255)
				);
		}
	}

	primitiveRenderer->popWorld();
}

void TranslateModifier::adjust(
	SceneEditorContext* context,
	const Matrix44& viewTransform,
	const Vector2& screenDelta,
	int button,
	Matrix44& outTransform
)
{
	Matrix44 world = Matrix44::identity();

	switch (context->getEditSpace())
	{
	case SceneEditorContext::EsWorld:
		world = translate(outTransform.translation());
		break;
	case SceneEditorContext::EsObject:
		world = outTransform;
		break;
	}
	
	int mouseX, mouseY, mouseZ;
	float scaleX, scaleY, scaleZ;
	getMouseDeltaAxises(
		world * viewTransform,
		mouseX, mouseY, mouseZ,
		scaleX, scaleY, scaleZ
	);

	Vector4 deltaX = Vector4::zero();
	Vector4 deltaY = Vector4::zero();
	Vector4 deltaZ = Vector4::zero();

	uint32_t axisEnable = context->getAxisEnable();

	if (axisEnable & SceneEditorContext::AeX)
		deltaX = c_axis[mouseX] * Scalar(-screenDelta.x * scaleX);
	if (axisEnable & SceneEditorContext::AeY)
		deltaY = c_axis[mouseY] * Scalar(screenDelta.y * scaleY);
	if (axisEnable & SceneEditorContext::AeZ)
		deltaZ = c_axis[mouseZ] * Scalar(screenDelta.y * scaleZ);

	Vector4 editDelta = (button == 0) ? (deltaX + deltaY) : (deltaX + deltaZ);

	switch (context->getEditSpace())
	{
	case SceneEditorContext::EsWorld:
		outTransform = outTransform * translate(editDelta);
		break;
	case SceneEditorContext::EsObject:
		outTransform = translate(editDelta) * outTransform;
		break;
	}
}

	}
}
