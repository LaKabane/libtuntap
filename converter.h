
#ifndef CONVERTER_H_
#define CONVERTER_H_
#include <jni.h>
#include <string.h>
#include <stdlib.h>

jstring charTojstring(JNIEnv* env, const char* pat);
char* jstringToChar(JNIEnv* env, jstring jstr);


#endif /* CONVERTER_H_ */
