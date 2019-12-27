/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#define LOG_TAG "MTKStringFactory"
#include <nativehelper/JNIHelp.h>
#include <nativehelper/JniConstants.h>
#include "JniException.h"
#include "jni.h"


static jstring MTKStringFactory_newStringFromUTF8Bytes(JNIEnv* env, jclass, jbyteArray java_data,
    jint offset, jint byte_count) {
  // Local Define in here
  static const jchar REPLACEMENT_CHAR = 0xfffd;
  static const int DEFAULT_BUFFER_SIZE = 256;
  static const int TABLE_UTF8_NEEDED[] = {
    //      0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xc0 - 0xcf
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xd0 - 0xdf
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0xe0 - 0xef
    3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xf0 - 0xff
  };

  if (java_data == nullptr && offset == 0 && byte_count == 0) {
    jniThrowExceptionFmt(env, "java/lang/StringIndexOutOfBoundsException",
        "offset=%d; byte_count=%d", offset, byte_count);
    return nullptr;
  }

  jbyte *jRawArray = env->GetByteArrayElements(java_data, NULL);
  if (jRawArray != nullptr) {
    // Initial value
    jchar temp_buffer[DEFAULT_BUFFER_SIZE];
    jbyte *d = jRawArray;
    jchar *v;
    bool v_need_free = false;
    if (byte_count <= DEFAULT_BUFFER_SIZE) {
      v = temp_buffer;
    } else {
      v = new jchar[byte_count];
      v_need_free = true;
    }

    int idx = offset;
    int last = offset + byte_count;
    int s = 0;

    int codePoint = 0;
    int utf8BytesSeen = 0;
    int utf8BytesNeeded = 0;
    int lowerBound = 0x80;
    int upperBound = 0xbf;
    while (idx < last) {
      int b = d[idx++] & 0xff;
      if (utf8BytesNeeded == 0) {
        if ((b & 0x80) == 0) { // ASCII char. 0xxxxxxx
          v[s++] = (jchar) b;
          continue;
        }

        if ((b & 0x40) == 0) { // 10xxxxxx is illegal as first byte
          v[s++] = REPLACEMENT_CHAR;
          continue;
        }

        // 11xxxxxx
        int tableLookupIndex = b & 0x3f;
        utf8BytesNeeded = TABLE_UTF8_NEEDED[tableLookupIndex];
        if (utf8BytesNeeded == 0) {
          v[s++] = REPLACEMENT_CHAR;
          continue;
        }

        // utf8BytesNeeded
        // 1: b & 0x1f
        // 2: b & 0x0f
        // 3: b & 0x07
        codePoint = b & (0x3f >> utf8BytesNeeded);
        if (b == 0xe0) {
          lowerBound = 0xa0;
        } else if (b == 0xed) {
          upperBound = 0x9f;
        } else if (b == 0xf0) {
          lowerBound = 0x90;
        } else if (b == 0xf4) {
          upperBound = 0x8f;
        }
      } else {
        if (b < lowerBound || b > upperBound) {
          // The bytes seen are ill-formed. Substitute them with U+FFFD
          v[s++] = REPLACEMENT_CHAR;
          codePoint = 0;
          utf8BytesNeeded = 0;
          utf8BytesSeen = 0;
          lowerBound = 0x80;
          upperBound = 0xbf;
          /*
           * According to the Unicode Standard,
           * "a UTF-8 conversion process is required to never consume well-formed
           * subsequences as part of its error handling for ill-formed subsequences"
           * The current byte could be part of well-formed subsequences. Reduce the
           * index by 1 to parse it in next loop.
           */
          idx--;
          continue;
        }

        lowerBound = 0x80;
        upperBound = 0xbf;
        codePoint = (codePoint << 6) | (b & 0x3f);
        utf8BytesSeen++;
        if (utf8BytesNeeded != utf8BytesSeen) {
          continue;
        }

        // Encode chars from U+10000 up as surrogate pairs
        if (codePoint < 0x10000) {
          v[s++] = (jchar) codePoint;
        } else {
          v[s++] = (jchar) ((codePoint >> 10) + 0xd7c0);
          v[s++] = (jchar) ((codePoint & 0x3ff) + 0xdc00);
        }

        utf8BytesSeen = 0;
        utf8BytesNeeded = 0;
        codePoint = 0;
      }
    }
    // The bytes seen are ill-formed. Substitute them by U+FFFD
    if (utf8BytesNeeded != 0) {
      v[s++] = REPLACEMENT_CHAR;
    }
    // Result handling
    // Release the orig. buffer
    env->ReleaseByteArrayElements(java_data, jRawArray, JNI_ABORT);
    if (env->ExceptionCheck() == JNI_TRUE) {
      if (v_need_free) {
        delete[] v;
      }
      return nullptr;
    }
    // Handle the new target string
    jstring rtn = env->NewString(v, s);

    // Free the template char array.
    if (v_need_free) {
      delete[] v;
    }
    return rtn;
  }

  return nullptr;
}


static JNINativeMethod gMethods[] = {
  NATIVE_METHOD(MTKStringFactory, newStringFromUTF8Bytes, "([BII)Ljava/lang/String;"),
};
void register_com_mediatek_libcore_MTKStringFactory(JNIEnv* env) {
  jniRegisterNativeMethods(env, "com/mediatek/libcore/MTKStringFactory", gMethods, NELEM(gMethods));
}
