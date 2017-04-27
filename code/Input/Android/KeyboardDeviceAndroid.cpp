/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <android/input.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include "Core/Log/Log.h"
#include "Input/Android/KeyboardDeviceAndroid.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

void displayKeyboard(ANativeActivity* activity, bool pShow)
{
	// Attaches the current thread to the JVM.
	jint lResult;
	jint lFlags = 0;

	JavaVM* lJavaVM = activity->vm;
	JNIEnv* lJNIEnv = activity->env;

	JavaVMAttachArgs lJavaVMAttachArgs;
	lJavaVMAttachArgs.version = JNI_VERSION_1_6;
	lJavaVMAttachArgs.name = "NativeThread";
	lJavaVMAttachArgs.group = NULL;

	lResult=lJavaVM->AttachCurrentThread(&lJNIEnv, &lJavaVMAttachArgs);
	if (lResult == JNI_ERR)
		return;

	// Retrieves NativeActivity.
	jobject lNativeActivity = activity->clazz;
	jclass ClassNativeActivity = lJNIEnv->GetObjectClass(lNativeActivity);

	// Retrieves Context.INPUT_METHOD_SERVICE.
	jclass ClassContext = lJNIEnv->FindClass("android/content/Context");
	jfieldID FieldINPUT_METHOD_SERVICE = lJNIEnv->GetStaticFieldID(ClassContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
	jobject INPUT_METHOD_SERVICE = lJNIEnv->GetStaticObjectField(ClassContext, FieldINPUT_METHOD_SERVICE);

	// Runs getSystemService(Context.INPUT_METHOD_SERVICE).
	jclass ClassInputMethodManager = lJNIEnv->FindClass(	"android/view/inputmethod/InputMethodManager");
	jmethodID MethodGetSystemService = lJNIEnv->GetMethodID(	ClassNativeActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
	jobject lInputMethodManager = lJNIEnv->CallObjectMethod(	lNativeActivity, MethodGetSystemService, INPUT_METHOD_SERVICE);

	// Runs getWindow().getDecorView().
	jmethodID MethodGetWindow = lJNIEnv->GetMethodID(ClassNativeActivity, "getWindow", "()Landroid/view/Window;");
	jobject lWindow = lJNIEnv->CallObjectMethod(lNativeActivity, MethodGetWindow);
	jclass ClassWindow = lJNIEnv->FindClass(	"android/view/Window");
	jmethodID MethodGetDecorView = lJNIEnv->GetMethodID(	ClassWindow, "getDecorView", "()Landroid/view/View;");
	jobject lDecorView = lJNIEnv->CallObjectMethod(lWindow, MethodGetDecorView);

	if (pShow)
	{
		// Runs lInputMethodManager.showSoftInput(...).
		jmethodID MethodShowSoftInput = lJNIEnv->GetMethodID(ClassInputMethodManager, "showSoftInput", "(Landroid/view/View;I)Z");
		lJNIEnv->CallBooleanMethod(lInputMethodManager, MethodShowSoftInput, lDecorView, lFlags);
	}
	else
	{
		// Runs lWindow.getViewToken()
		jclass ClassView = lJNIEnv->FindClass("android/view/View");
		jmethodID MethodGetWindowToken = lJNIEnv->GetMethodID(ClassView, "getWindowToken", "()Landroid/os/IBinder;");
		jobject lBinder = lJNIEnv->CallObjectMethod(lDecorView, MethodGetWindowToken);

		// lInputMethodManager.hideSoftInput(...).
		jmethodID MethodHideSoftInput = lJNIEnv->GetMethodID(ClassInputMethodManager, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
		lJNIEnv->CallBooleanMethod(lInputMethodManager, MethodHideSoftInput, lBinder, lFlags);
	}

	// Finished with the JVM.
	lJavaVM->DetachCurrentThread();
}

int getUnicodeChar(ANativeActivity* activity, int eventType, int keyCode, int metaState)
{
	// \hack Intercept backspace keycode.
	if (keyCode == 67)
		return L'\b';

	JavaVM* javaVM = activity->vm;
	JNIEnv* jniEnv = activity->env;

	JavaVMAttachArgs attachArgs;
	attachArgs.version = JNI_VERSION_1_6;
	attachArgs.name = "NativeThread";
	attachArgs.group = NULL;

	jint result = javaVM->AttachCurrentThread(&jniEnv, &attachArgs);
	if (result == JNI_ERR)
		return 0;

	jclass class_key_event = jniEnv->FindClass("android/view/KeyEvent");
	int unicodeKey;

	if (metaState == 0)
	{
		jmethodID method_get_unicode_char = jniEnv->GetMethodID(class_key_event, "getUnicodeChar", "()I");
		jmethodID eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>", "(II)V");
		jobject eventObj = jniEnv->NewObject(class_key_event, eventConstructor, eventType, keyCode);
		unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char);
	}
	else
	{
		jmethodID method_get_unicode_char = jniEnv->GetMethodID(class_key_event, "getUnicodeChar", "(I)I");
		jmethodID eventConstructor = jniEnv->GetMethodID(class_key_event, "<init>", "(II)V");
		jobject eventObj = jniEnv->NewObject(class_key_event, eventConstructor, eventType, keyCode);
		unicodeKey = jniEnv->CallIntMethod(eventObj, method_get_unicode_char, metaState);
	}

	javaVM->DetachCurrentThread();
	return unicodeKey;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardDeviceAndroid", KeyboardDeviceAndroid, IInputDevice)

struct ANativeActivity* KeyboardDeviceAndroid::ms_activity = 0;

KeyboardDeviceAndroid::KeyboardDeviceAndroid()
{
	resetState();
}

std::wstring KeyboardDeviceAndroid::getName() const
{
	return L"Keyboard";
}

InputCategory KeyboardDeviceAndroid::getCategory() const
{
	return CtKeyboard;
}

bool KeyboardDeviceAndroid::isConnected() const
{
	return true;
}

int32_t KeyboardDeviceAndroid::getControlCount()
{
	return 0;
}

std::wstring KeyboardDeviceAndroid::getControlName(int32_t control)
{
	return L"";
}

bool KeyboardDeviceAndroid::isControlAnalogue(int32_t control) const
{
	return false;
}

bool KeyboardDeviceAndroid::isControlStable(int32_t control) const
{
	return true;
}

float KeyboardDeviceAndroid::getControlValue(int32_t control)
{
	return 0.0f;
}

bool KeyboardDeviceAndroid::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool KeyboardDeviceAndroid::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	return false;
}

bool KeyboardDeviceAndroid::getKeyEvent(KeyEvent& outEvent)
{
	if (m_keyEvents.empty())
		return false;

	outEvent = m_keyEvents.front();
	m_keyEvents.pop_front();
	
	return true;
}

void KeyboardDeviceAndroid::resetState()
{
}

void KeyboardDeviceAndroid::readState()
{
}

bool KeyboardDeviceAndroid::supportRumble() const
{
	return false;
}

void KeyboardDeviceAndroid::setRumble(const InputRumble& /*rumble*/)
{
}

void KeyboardDeviceAndroid::setExclusive(bool exclusive)
{
}

void KeyboardDeviceAndroid::showSoftKeyboard()
{
	displayKeyboard(ms_activity, true);
}

void KeyboardDeviceAndroid::hideSoftKeyboard()
{
	displayKeyboard(ms_activity, false);
}

void KeyboardDeviceAndroid::handleInput(AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_KEY)
		return;

	int32_t action = AKeyEvent_getAction(event);
	KeyEvent ke;

	if (action == AKEY_EVENT_ACTION_DOWN)
	{
		int32_t key = AKeyEvent_getKeyCode(event);
		int32_t meta = AKeyEvent_getMetaState(event);
		wchar_t ch = getUnicodeChar(ms_activity, AKEY_EVENT_ACTION_DOWN, key, meta);
		
		ke.type = KtDown;
		ke.keyCode = key;
		ke.character = ch;
		m_keyEvents.push_back(ke);

		if (ch)
		{
			ke.type = KtCharacter;
			ke.keyCode = key;
			ke.character = ch;
			m_keyEvents.push_back(ke);
		}
	}
	else if (action == AKEY_EVENT_ACTION_UP)
	{
		int32_t key = AKeyEvent_getKeyCode(event);
		int32_t meta = AKeyEvent_getMetaState(event);
		wchar_t ch = getUnicodeChar(ms_activity, AKEY_EVENT_ACTION_DOWN, key, meta);

		ke.type = KtUp;
		ke.keyCode = key;
		ke.character = ch;
		m_keyEvents.push_back(ke);
	}
}

	}
}
