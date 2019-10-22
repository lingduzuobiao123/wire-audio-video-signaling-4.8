/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <android/log.h>

#include <memory>

#include "webrtc/modules/utility/include/jvm_android.h"

#include "webrtc/base/checks.h"

#define TAG "JVM"
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace webrtc {

JVM* g_jvm;

static struct {
       JavaVM *vm;
} java;

struct jni_env {
       bool attached;
       JNIEnv *env;
};

static int jni_attach(struct jni_env *je)
{
       int res;
       int err = 0;

       je->attached = false;
       res = java.vm->GetEnv((void **)&je->env, JNI_VERSION_1_6);

       if (res != JNI_OK || je->env == NULL) {
               res = java.vm->AttachCurrentThread(&je->env, NULL);
	       ALOGD("jni_attach: returned: %d OK=%d\n", res, JNI_OK);
               if (res != JNI_OK) {
                       err = ENOSYS;
                       goto out;
               }
               je->attached = true;
       }

out:
      return err;
}

static void jni_detach(struct jni_env *je)
{
       if (je->attached) {
               java.vm->DetachCurrentThread();
       }
}

// TODO(henrika): add more clases here if needed.
struct {
  const char* name;
  jclass clazz;
} loaded_classes[] = {
  {"org/webrtc/voiceengine/BuildInfo", nullptr},
  {"org/webrtc/voiceengine/WebRtcAudioManager", nullptr},
  {"org/webrtc/voiceengine/WebRtcAudioRecord", nullptr},
  {"org/webrtc/voiceengine/WebRtcAudioTrack", nullptr},
};

// Android's FindClass() is trickier than usual because the app-specific
// ClassLoader is not consulted when there is no app-specific frame on the
// stack.  Consequently, we only look up all classes once in native WebRTC.
// http://developer.android.com/training/articles/perf-jni.html#faq_FindClass
void LoadClasses(JNIEnv* jni) {
  for (auto& c : loaded_classes) {
    jclass localRef = FindClass(jni, c.name);
    CHECK_EXCEPTION(jni) << "Error during FindClass: " << c.name;
    RTC_CHECK(localRef) << c.name;
    jclass globalRef = reinterpret_cast<jclass>(jni->NewGlobalRef(localRef));
    CHECK_EXCEPTION(jni) << "Error during NewGlobalRef: " << c.name;
    RTC_CHECK(globalRef) << c.name;
    c.clazz = globalRef;
  }
}

void FreeClassReferences(JNIEnv* jni) {
  for (auto& c : loaded_classes) {
    jni->DeleteGlobalRef(c.clazz);
    c.clazz = nullptr;
  }
}

jclass LookUpClass(const char* name) {
  for (auto& c : loaded_classes) {
    if (strcmp(c.name, name) == 0)
      return c.clazz;
  }
  RTC_CHECK(false) << "Unable to find class in lookup table";
  return 0;
}

// AttachCurrentThreadIfNeeded implementation.
AttachCurrentThreadIfNeeded::AttachCurrentThreadIfNeeded()
    : attached_(false) {
#if 0
  ALOGD("AttachCurrentThreadIfNeeded::ctor%s", GetThreadInfo().c_str());
  JavaVM* jvm = JVM::GetInstance()->jvm();
  RTC_CHECK(jvm);
  JNIEnv* jni = GetEnv(jvm);
  if (!jni) {
    ALOGD("Attaching thread to JVM");
    JNIEnv* env = nullptr;
    jint ret = jvm->AttachCurrentThread(&env, nullptr);
    attached_ = (ret == JNI_OK);
  }
#endif
}

AttachCurrentThreadIfNeeded::~AttachCurrentThreadIfNeeded() {
#if 0
  ALOGD("AttachCurrentThreadIfNeeded::dtor%s", GetThreadInfo().c_str());
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  if (attached_) {
    ALOGD("Detaching thread from JVM");
    jint res = JVM::GetInstance()->jvm()->DetachCurrentThread();
    RTC_CHECK(res == JNI_OK) << "DetachCurrentThread failed: " << res;
  }
#endif
}

// GlobalRef implementation.
GlobalRef::GlobalRef(JNIEnv *jni, jobject obj)
{
      struct jni_env je;
      bool attach = false;

      if (jni == NULL) {
              jni_attach(&je);
	      jni = je.env;
	      attach = true;
      }
      j_object_ = jni->NewGlobalRef(obj);

      if (attach)
	      jni_detach(&je);

      ALOGD("GlobalRef::ctor%s", GetThreadInfo().c_str());
}

GlobalRef::~GlobalRef() {
       ALOGD("GlobalRef::dtor%s o=%p", GetThreadInfo().c_str(), j_object_);
       struct jni_env je;
       jni_attach(&je);
       je.env->DeleteGlobalRef(j_object_);
       jni_detach(&je);
}

jboolean GlobalRef::CallBooleanMethod(jmethodID methodID, ...) {
  va_list args;
  jboolean res;
  va_start(args, methodID);
  {
         struct jni_env je;

         jni_attach(&je);
         res = je.env->CallBooleanMethodV(j_object_, methodID, args);
         CHECK_EXCEPTION(je.env) << "Error during CallBooleanMethod";
         jni_detach(&je);
  }

  va_end(args);
  return res;
}

jint GlobalRef::CallIntMethod(jmethodID methodID, ...) {
  va_list args;
  jint res;
  va_start(args, methodID);
  {
         struct jni_env je;
         jni_attach(&je);
         res = je.env->CallIntMethodV(j_object_, methodID, args);
         CHECK_EXCEPTION(je.env) << "Error during CallIntMethod";
         jni_detach(&je);
  }
  va_end(args);
  return res;
}

void GlobalRef::CallVoidMethod(jmethodID methodID, ...) {
  va_list args;
  va_start(args, methodID);
  {
         struct jni_env je;
         jni_attach(&je);
         je.env->CallVoidMethodV(j_object_, methodID, args);
         CHECK_EXCEPTION(je.env) << "Error during CallVoidMethod";
         jni_detach(&je);
  }
  va_end(args);
}

// NativeRegistration implementation.
NativeRegistration::NativeRegistration(jclass clazz)
    : JavaClass(clazz) {
  ALOGD("NativeRegistration::ctor%s", GetThreadInfo().c_str());
}

NativeRegistration::~NativeRegistration() {
  ALOGD("NativeRegistration::dtor%s", GetThreadInfo().c_str());

  struct jni_env je;

  jni_attach(&je);
  je.env->UnregisterNatives(j_class_);
  CHECK_EXCEPTION(je.env) << "Error during UnregisterNatives";
  jni_detach(&je);
}

std::unique_ptr<GlobalRef> NativeRegistration::NewObject(
    const char* name, const char* signature, ...) {
  ALOGD("NativeRegistration::NewObject%s", GetThreadInfo().c_str());
  va_list args;
  jobject obj;
  jmethodID mid;
  struct jni_env je;

  va_start(args, signature);

  jni_attach(&je);
  mid = je.env->GetMethodID(j_class_, name, signature);
  obj = je.env->NewObjectV(j_class_, mid, args);

  va_end(args);

  ALOGD("NativeRegistration::NewObject: mid=%p obj=%p", mid, obj);

  GlobalRef *gref = new GlobalRef(je.env, obj);
  CHECK_EXCEPTION(je.env) << "Error during NewObjectV";
  jni_detach(&je);

  return std::unique_ptr<GlobalRef>(gref);
}

// JavaClass implementation.
jmethodID JavaClass::GetMethodId(
    const char* name, const char* signature) {
	jmethodID mid;
	struct jni_env je;

	jni_attach(&je);
	mid = je.env->GetMethodID(j_class_, name, signature);
	jni_detach(&je);

	return mid;
}

jmethodID JavaClass::GetStaticMethodId(
    const char* name, const char* signature) {
      jmethodID mid;
      struct jni_env je;

      jni_attach(&je);
      je.env->GetStaticMethodID(j_class_, name, signature);
      jni_detach(&je);

      return mid;
}

jobject JavaClass::CallStaticObjectMethod(jmethodID methodID, ...) {
  va_list args;
  va_start(args, methodID);
  jobject res;
  {
	  struct jni_env je;
	  jni_attach(&je);

	  res = je.env->CallStaticObjectMethod(j_class_, methodID, args);
	  CHECK_EXCEPTION(je.env) << "Error during CallStaticObjectMethod";
	  jni_detach(&je);
  }
  return res;
}

std::unique_ptr<NativeRegistration> JNIEnvironment::RegisterNatives(
    const char* name, const JNINativeMethod *methods, int num_methods) {
  ALOGD("JNIEnvironment::RegisterNatives(%s)", name);
  jclass clazz = LookUpClass(name);
  struct jni_env je;

  jni_attach(&je);
  je.env->RegisterNatives(clazz, methods, num_methods);
  CHECK_EXCEPTION(je.env) << "Error during RegisterNatives";
  jni_detach(&je);

  return std::unique_ptr<NativeRegistration>(new NativeRegistration(clazz));
}

std::string JNIEnvironment::JavaToStdString(const jstring& j_string) {
	struct jni_env je;

	jni_attach(&je);
	const char* jchars = je.env->GetStringUTFChars(j_string, nullptr);
	CHECK_EXCEPTION(je.env);
	const int size = je.env->GetStringUTFLength(j_string);
	CHECK_EXCEPTION(je.env);

	std::string ret(jchars, size);
	je.env->ReleaseStringUTFChars(j_string, jchars);
	CHECK_EXCEPTION(je.env);
	jni_detach(&je);

	return ret;
}

// static
void JVM::Initialize(JavaVM* jvm, jobject context) {
  ALOGD("JVM::Initialize%s", GetThreadInfo().c_str());
  java.vm = jvm;
  RTC_CHECK(!g_jvm);
  g_jvm = new JVM(jvm, context);
}

// static
void JVM::Uninitialize() {
  ALOGD("JVM::Uninitialize%s", GetThreadInfo().c_str());
  RTC_DCHECK(g_jvm);
  delete g_jvm;
  g_jvm = nullptr;
}

// static
JVM* JVM::GetInstance() {
  RTC_DCHECK(g_jvm);
  return g_jvm;
}

JVM::JVM(JavaVM* jvm, jobject context)
    : jvm_(jvm) {
	ALOGD("JVM::JVM%s", GetThreadInfo().c_str());

	struct jni_env je;

	jni_attach(&je);
	context_ = je.env->NewGlobalRef(context);
	LoadClasses(je.env);
	jni_detach(&je);
}

JVM::~JVM() {
  struct jni_env je;

  ALOGD("JVM::~JVM%s", GetThreadInfo().c_str());

  jni_attach(&je);
  FreeClassReferences(je.env);
  je.env->DeleteGlobalRef(context_);
  jni_detach(&je);
}

std::unique_ptr<JNIEnvironment> JVM::environment() {
	return NULL;
}

JavaClass JVM::GetClass(const char* name) {
  ALOGD("JVM::GetClass(%s)%s", name, GetThreadInfo().c_str());
  RTC_DCHECK(thread_checker_.CalledOnValidThread());
  return JavaClass(LookUpClass(name));
}

}  // namespace webrtc
