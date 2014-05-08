#include <cpu-features.h>
#include <jni.h>

extern "C"
{

jboolean Java_com_qingping_EmulatorActivity_haveNeon(JNIEnv* env, jclass* clazz)
{
	AndroidCpuFamily cpuFamily = android_getCpuFamily();
	uint64_t cpuFeatures = android_getCpuFeatures();
	if (cpuFamily == ANDROID_CPU_FAMILY_ARM &&
        (cpuFeatures & ANDROID_CPU_ARM_FEATURE_NEON) != 0)
    {
		return true;
    }

	return false;
}

}
