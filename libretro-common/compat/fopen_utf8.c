/* Copyright  (C) 2010-2020 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (fopen_utf8.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <compat/fopen_utf8.h>
#include <retro_miscellaneous.h>
#include <encodings/utf.h>
#include <stdio.h>
#include <stdlib.h>
#include <fileapi.h>

#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0500 || defined(_XBOX)
#ifndef LEGACY_WIN32
#define LEGACY_WIN32
#endif
#endif

#ifdef _WIN32
#undef fopen

void *fopen_utf8(const char * filename, const char * mode)
{
#if defined(LEGACY_WIN32)
   FILE             *ret = NULL;
   char * filename_local = utf8_to_local_string_alloc(filename);

   if (!filename_local)
      return NULL;
   ret = fopen(filename_local, mode);
   if (filename_local)
      free(filename_local);
   return ret;
#else
   wchar_t* windows_long_prefix = NULL;
   wchar_t* filename_w = NULL;
   wchar_t* print_format_w = NULL;

   wchar_t filename_w_prefix[PATH_MAX_LENGTH];
   wchar_t filename_w_full[PATH_MAX_LENGTH];
   wchar_t filename_w_short[PATH_MAX_LENGTH];

   filename_w = utf8_to_utf16_string_alloc(filename);

   if (strlen(filename) >= MAX_PATH) {
      /*
       trick to bypass the ~260 characters limit in many Windows I/O APIs
       http://arsenmk.blogspot.com/2015/12/handling-long-paths-on-windows.html
      */

      /* try get the full path from the input path */
      if (GetFullPathNameW(filename_w, PATH_MAX_LENGTH, filename_w_full, NULL) == 0) {
         if (filename_w)
            free(filename_w);
         return NULL;
      }

      /* try to know if we are dealing with a short (format) path */
      if (GetShortPathNameW(filename_w_full, filename_w_short, PATH_MAX_LENGTH)) {
         int cmp_result = wcscmp(filename_w_full, filename_w_short);
         if (cmp_result == 0) {
            /* yes, short path => full path */
            GetLongPathNameW(filename_w_short, filename_w_full, PATH_MAX_LENGTH);
         }
      }

      /* the full path is a network share path ie '\\<machine-name>\<share-path>\<filename>' */
      if (filename_w_full[0] == '\\' && filename_w_full[1] == '\\') {
         windows_long_prefix = utf8_to_utf16_string_alloc("\\\\?\\UNC");
      }
      /* the full path is a regular local path */
      else {
         windows_long_prefix = utf8_to_utf16_string_alloc("\\\\?\\");
      }

      /* our final full, long path */
      print_format_w = utf8_to_utf16_string_alloc("%s%s");
      _snwprintf(filename_w_prefix, PATH_MAX_LENGTH, print_format_w, windows_long_prefix, filename_w_full);
      if (filename_w)
         free(filename_w);
      filename_w = filename_w_full;
   }

   wchar_t * mode_w = utf8_to_utf16_string_alloc(mode);
   FILE* ret = NULL;

   if (filename_w && mode_w) {
      ret = _wfopen(filename_w, mode_w);
   }
   if (filename_w)
      free(filename_w);
   if (windows_long_prefix)
      free(windows_long_prefix);
   if (print_format_w)
      free(print_format_w);
   if (mode_w)
      free(mode_w);
   return ret;
#endif /* if LEGACY_WIN32 else */
}
#endif /* _WIN32 */
