/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include "jni.h"
#include "jvm.h"

#include <nativehelper/JNIHelp.h>

#define NATIVE_METHOD(className, functionName, signature) \
{ #functionName, signature, (void*)(className ## _ ## functionName) }


/*
 * Find the float corresponding to a given bit pattern
 */
JNIEXPORT jfloat JNICALL
Float_intBitsToFloat(JNIEnv *env, jclass unused, jint v)
{
    union {
        int i;
        float f;
    } u;
    u.i = (long)v;
    return (jfloat)u.f;
}

/*
 * Find the bit pattern corresponding to a given float, NOT collapsing NaNs
 */
JNIEXPORT jint JNICALL
Float_floatToRawIntBits(JNIEnv *env, jclass unused, jfloat v)
{
    union {
        int i;
        float f;
    } u;
    u.f = (float)v;
    return (jint)u.i;
}
static JNINativeMethod gMethods[] = {
  NATIVE_METHOD(Float, intBitsToFloat, "(I)F"),
  NATIVE_METHOD(Float, floatToRawIntBits, "(F)I"),
};

void register_java_lang_Float(JNIEnv* env) {
  jniRegisterNativeMethods(env, "java/lang/Float", gMethods, NELEM(gMethods));
}
