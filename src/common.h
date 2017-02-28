/*
 * Copyright 2016 WebAssembly Community Group participants
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WABT_COMMON_H_
#define WABT_COMMON_H_

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#ifdef __cplusplus
#define WABT_EXTERN_C extern "C"
#define WABT_EXTERN_C_BEGIN extern "C" {
#define WABT_EXTERN_C_END }
#else
#define WABT_EXTERN_C
#define WABT_EXTERN_C_BEGIN
#define WABT_EXTERN_C_END
#endif

#define WABT_FATAL(...) fprintf(stderr, __VA_ARGS__), exit(1)
#define WABT_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define WABT_ZERO_MEMORY(var) memset(static_cast<void*>(&(var)), 0, sizeof(var))
#define WABT_USE(x) static_cast<void>(x)

#define WABT_UNKNOWN_OFFSET (static_cast<uint32_t>(~0))
#define WABT_PAGE_SIZE 0x10000 /* 64k */
#define WABT_MAX_PAGES 0x10000 /* # of pages that fit in 32-bit address space */
#define WABT_BYTES_TO_PAGES(x) ((x) >> 16)
#define WABT_ALIGN_UP_TO_PAGE(x) \
  (((x) + WABT_PAGE_SIZE - 1) & ~(WABT_PAGE_SIZE - 1))

#define PRIstringslice "%.*s"
#define WABT_PRINTF_STRING_SLICE_ARG(x) static_cast<int>((x).length), (x).start

#define WABT_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE 128
#define WABT_SNPRINTF_ALLOCA(buffer, len, format)                          \
  va_list args;                                                            \
  va_list args_copy;                                                       \
  va_start(args, format);                                                  \
  va_copy(args_copy, args);                                                \
  char fixed_buf[WABT_DEFAULT_SNPRINTF_ALLOCA_BUFSIZE];                    \
  char* buffer = fixed_buf;                                                \
  size_t len = wabt_vsnprintf(fixed_buf, sizeof(fixed_buf), format, args); \
  va_end(args);                                                            \
  if (len + 1 > sizeof(fixed_buf)) {                                       \
    buffer = static_cast<char*>(alloca(len + 1));                          \
    len = wabt_vsnprintf(buffer, len + 1, format, args_copy);              \
  }                                                                        \
  va_end(args_copy)

enum WabtResult {
  WABT_OK,
  WABT_ERROR,
};

#define WABT_SUCCEEDED(x) ((x) == WABT_OK)
#define WABT_FAILED(x) ((x) == WABT_ERROR)

enum WabtLabelType {
  WABT_LABEL_TYPE_FUNC,
  WABT_LABEL_TYPE_BLOCK,
  WABT_LABEL_TYPE_LOOP,
  WABT_LABEL_TYPE_IF,
  WABT_LABEL_TYPE_ELSE,
  WABT_NUM_LABEL_TYPES,
};

struct WabtStringSlice {
  const char* start;
  size_t length;
};

struct WabtLocation {
  const char* filename;
  int line;
  int first_column;
  int last_column;
};

typedef void (*WabtSourceErrorCallback)(const WabtLocation*,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data);

struct WabtSourceErrorHandler {
  WabtSourceErrorCallback on_error;
  /* on_error will be called with with source_line trimmed to this length */
  size_t source_line_max_length;
  void* user_data;
};

#define WABT_SOURCE_LINE_MAX_LENGTH_DEFAULT 80
#define WABT_SOURCE_ERROR_HANDLER_DEFAULT                                    \
  {                                                                          \
    wabt_default_source_error_callback, WABT_SOURCE_LINE_MAX_LENGTH_DEFAULT, \
        nullptr                                                              \
  }

typedef void (*WabtBinaryErrorCallback)(uint32_t offset,
                                        const char* error,
                                        void* user_data);

struct WabtBinaryErrorHandler {
  WabtBinaryErrorCallback on_error;
  void* user_data;
};

#define WABT_BINARY_ERROR_HANDLER_DEFAULT \
  { wabt_default_binary_error_callback, nullptr }

/* This data structure is not required; it is just used by the default error
 * handler callbacks. */
enum WabtPrintErrorHeader {
  WABT_PRINT_ERROR_HEADER_NEVER,
  WABT_PRINT_ERROR_HEADER_ONCE,
  WABT_PRINT_ERROR_HEADER_ALWAYS,
};

struct WabtDefaultErrorHandlerInfo {
  const char* header;
  FILE* out_file;
  WabtPrintErrorHeader print_header;
};

/* matches binary format, do not change */
enum WabtType {
  WABT_TYPE_I32 = -0x01,
  WABT_TYPE_I64 = -0x02,
  WABT_TYPE_F32 = -0x03,
  WABT_TYPE_F64 = -0x04,
  WABT_TYPE_ANYFUNC = -0x10,
  WABT_TYPE_FUNC = -0x20,
  WABT_TYPE_VOID = -0x40,
  WABT_TYPE____ = WABT_TYPE_VOID, /* convenient for the opcode table below */
  WABT_TYPE_ANY = 0, /* Not actually specified, but useful for type-checking */
};

enum WabtRelocType {
  WABT_RELOC_FUNC_INDEX_LEB = 0,   /* e.g. immediate of call instruction */
  WABT_RELOC_TABLE_INDEX_SLEB = 1, /* e.g. loading address of function */
  WABT_RELOC_TABLE_INDEX_I32 = 2,  /* e.g. function address in DATA */
  WABT_RELOC_GLOBAL_INDEX_LEB = 3, /* e.g immediate of get_global inst */
  WABT_RELOC_DATA = 4,
  WABT_NUM_RELOC_TYPES,
};

/* matches binary format, do not change */
enum WabtExternalKind {
  WABT_EXTERNAL_KIND_FUNC = 0,
  WABT_EXTERNAL_KIND_TABLE = 1,
  WABT_EXTERNAL_KIND_MEMORY = 2,
  WABT_EXTERNAL_KIND_GLOBAL = 3,
  WABT_NUM_EXTERNAL_KINDS,
};

struct WabtLimits {
  uint64_t initial;
  uint64_t max;
  bool has_max;
};

enum { WABT_USE_NATURAL_ALIGNMENT = 0xFFFFFFFF };

/*
 *   tr: result type
 *   t1: type of the 1st parameter
 *   t2: type of the 2nd parameter
 *    m: memory size of the operation, if any
 * code: opcode
 * NAME: used to generate the opcode enum
 * text: a string of the opcode name in the AST format
 *
 *  tr  t1    t2   m  code  NAME text
 *  ============================ */
#define WABT_FOREACH_OPCODE(V)                                          \
  V(___, ___, ___, 0, 0x00, UNREACHABLE, "unreachable")                 \
  V(___, ___, ___, 0, 0x01, NOP, "nop")                                 \
  V(___, ___, ___, 0, 0x02, BLOCK, "block")                             \
  V(___, ___, ___, 0, 0x03, LOOP, "loop")                               \
  V(___, ___, ___, 0, 0x04, IF, "if")                                   \
  V(___, ___, ___, 0, 0x05, ELSE, "else")                               \
  V(___, ___, ___, 0, 0x0b, END, "end")                                 \
  V(___, ___, ___, 0, 0x0c, BR, "br")                                   \
  V(___, ___, ___, 0, 0x0d, BR_IF, "br_if")                             \
  V(___, ___, ___, 0, 0x0e, BR_TABLE, "br_table")                       \
  V(___, ___, ___, 0, 0x0f, RETURN, "return")                           \
  V(___, ___, ___, 0, 0x10, CALL, "call")                               \
  V(___, ___, ___, 0, 0x11, CALL_INDIRECT, "call_indirect")             \
  V(___, ___, ___, 0, 0x1a, DROP, "drop")                               \
  V(___, ___, ___, 0, 0x1b, SELECT, "select")                           \
  V(___, ___, ___, 0, 0x20, GET_LOCAL, "get_local")                     \
  V(___, ___, ___, 0, 0x21, SET_LOCAL, "set_local")                     \
  V(___, ___, ___, 0, 0x22, TEE_LOCAL, "tee_local")                     \
  V(___, ___, ___, 0, 0x23, GET_GLOBAL, "get_global")                   \
  V(___, ___, ___, 0, 0x24, SET_GLOBAL, "set_global")                   \
  V(I32, I32, ___, 4, 0x28, I32_LOAD, "i32.load")                       \
  V(I64, I32, ___, 8, 0x29, I64_LOAD, "i64.load")                       \
  V(F32, I32, ___, 4, 0x2a, F32_LOAD, "f32.load")                       \
  V(F64, I32, ___, 8, 0x2b, F64_LOAD, "f64.load")                       \
  V(I32, I32, ___, 1, 0x2c, I32_LOAD8_S, "i32.load8_s")                 \
  V(I32, I32, ___, 1, 0x2d, I32_LOAD8_U, "i32.load8_u")                 \
  V(I32, I32, ___, 2, 0x2e, I32_LOAD16_S, "i32.load16_s")               \
  V(I32, I32, ___, 2, 0x2f, I32_LOAD16_U, "i32.load16_u")               \
  V(I64, I32, ___, 1, 0x30, I64_LOAD8_S, "i64.load8_s")                 \
  V(I64, I32, ___, 1, 0x31, I64_LOAD8_U, "i64.load8_u")                 \
  V(I64, I32, ___, 2, 0x32, I64_LOAD16_S, "i64.load16_s")               \
  V(I64, I32, ___, 2, 0x33, I64_LOAD16_U, "i64.load16_u")               \
  V(I64, I32, ___, 4, 0x34, I64_LOAD32_S, "i64.load32_s")               \
  V(I64, I32, ___, 4, 0x35, I64_LOAD32_U, "i64.load32_u")               \
  V(___, I32, I32, 4, 0x36, I32_STORE, "i32.store")                     \
  V(___, I32, I64, 8, 0x37, I64_STORE, "i64.store")                     \
  V(___, I32, F32, 4, 0x38, F32_STORE, "f32.store")                     \
  V(___, I32, F64, 8, 0x39, F64_STORE, "f64.store")                     \
  V(___, I32, I32, 1, 0x3a, I32_STORE8, "i32.store8")                   \
  V(___, I32, I32, 2, 0x3b, I32_STORE16, "i32.store16")                 \
  V(___, I32, I64, 1, 0x3c, I64_STORE8, "i64.store8")                   \
  V(___, I32, I64, 2, 0x3d, I64_STORE16, "i64.store16")                 \
  V(___, I32, I64, 4, 0x3e, I64_STORE32, "i64.store32")                 \
  V(I32, ___, ___, 0, 0x3f, CURRENT_MEMORY, "current_memory")           \
  V(I32, I32, ___, 0, 0x40, GROW_MEMORY, "grow_memory")                 \
  V(I32, ___, ___, 0, 0x41, I32_CONST, "i32.const")                     \
  V(I64, ___, ___, 0, 0x42, I64_CONST, "i64.const")                     \
  V(F32, ___, ___, 0, 0x43, F32_CONST, "f32.const")                     \
  V(F64, ___, ___, 0, 0x44, F64_CONST, "f64.const")                     \
  V(I32, I32, ___, 0, 0x45, I32_EQZ, "i32.eqz")                         \
  V(I32, I32, I32, 0, 0x46, I32_EQ, "i32.eq")                           \
  V(I32, I32, I32, 0, 0x47, I32_NE, "i32.ne")                           \
  V(I32, I32, I32, 0, 0x48, I32_LT_S, "i32.lt_s")                       \
  V(I32, I32, I32, 0, 0x49, I32_LT_U, "i32.lt_u")                       \
  V(I32, I32, I32, 0, 0x4a, I32_GT_S, "i32.gt_s")                       \
  V(I32, I32, I32, 0, 0x4b, I32_GT_U, "i32.gt_u")                       \
  V(I32, I32, I32, 0, 0x4c, I32_LE_S, "i32.le_s")                       \
  V(I32, I32, I32, 0, 0x4d, I32_LE_U, "i32.le_u")                       \
  V(I32, I32, I32, 0, 0x4e, I32_GE_S, "i32.ge_s")                       \
  V(I32, I32, I32, 0, 0x4f, I32_GE_U, "i32.ge_u")                       \
  V(I32, I64, ___, 0, 0x50, I64_EQZ, "i64.eqz")                         \
  V(I32, I64, I64, 0, 0x51, I64_EQ, "i64.eq")                           \
  V(I32, I64, I64, 0, 0x52, I64_NE, "i64.ne")                           \
  V(I32, I64, I64, 0, 0x53, I64_LT_S, "i64.lt_s")                       \
  V(I32, I64, I64, 0, 0x54, I64_LT_U, "i64.lt_u")                       \
  V(I32, I64, I64, 0, 0x55, I64_GT_S, "i64.gt_s")                       \
  V(I32, I64, I64, 0, 0x56, I64_GT_U, "i64.gt_u")                       \
  V(I32, I64, I64, 0, 0x57, I64_LE_S, "i64.le_s")                       \
  V(I32, I64, I64, 0, 0x58, I64_LE_U, "i64.le_u")                       \
  V(I32, I64, I64, 0, 0x59, I64_GE_S, "i64.ge_s")                       \
  V(I32, I64, I64, 0, 0x5a, I64_GE_U, "i64.ge_u")                       \
  V(I32, F32, F32, 0, 0x5b, F32_EQ, "f32.eq")                           \
  V(I32, F32, F32, 0, 0x5c, F32_NE, "f32.ne")                           \
  V(I32, F32, F32, 0, 0x5d, F32_LT, "f32.lt")                           \
  V(I32, F32, F32, 0, 0x5e, F32_GT, "f32.gt")                           \
  V(I32, F32, F32, 0, 0x5f, F32_LE, "f32.le")                           \
  V(I32, F32, F32, 0, 0x60, F32_GE, "f32.ge")                           \
  V(I32, F64, F64, 0, 0x61, F64_EQ, "f64.eq")                           \
  V(I32, F64, F64, 0, 0x62, F64_NE, "f64.ne")                           \
  V(I32, F64, F64, 0, 0x63, F64_LT, "f64.lt")                           \
  V(I32, F64, F64, 0, 0x64, F64_GT, "f64.gt")                           \
  V(I32, F64, F64, 0, 0x65, F64_LE, "f64.le")                           \
  V(I32, F64, F64, 0, 0x66, F64_GE, "f64.ge")                           \
  V(I32, I32, ___, 0, 0x67, I32_CLZ, "i32.clz")                         \
  V(I32, I32, ___, 0, 0x68, I32_CTZ, "i32.ctz")                         \
  V(I32, I32, ___, 0, 0x69, I32_POPCNT, "i32.popcnt")                   \
  V(I32, I32, I32, 0, 0x6a, I32_ADD, "i32.add")                         \
  V(I32, I32, I32, 0, 0x6b, I32_SUB, "i32.sub")                         \
  V(I32, I32, I32, 0, 0x6c, I32_MUL, "i32.mul")                         \
  V(I32, I32, I32, 0, 0x6d, I32_DIV_S, "i32.div_s")                     \
  V(I32, I32, I32, 0, 0x6e, I32_DIV_U, "i32.div_u")                     \
  V(I32, I32, I32, 0, 0x6f, I32_REM_S, "i32.rem_s")                     \
  V(I32, I32, I32, 0, 0x70, I32_REM_U, "i32.rem_u")                     \
  V(I32, I32, I32, 0, 0x71, I32_AND, "i32.and")                         \
  V(I32, I32, I32, 0, 0x72, I32_OR, "i32.or")                           \
  V(I32, I32, I32, 0, 0x73, I32_XOR, "i32.xor")                         \
  V(I32, I32, I32, 0, 0x74, I32_SHL, "i32.shl")                         \
  V(I32, I32, I32, 0, 0x75, I32_SHR_S, "i32.shr_s")                     \
  V(I32, I32, I32, 0, 0x76, I32_SHR_U, "i32.shr_u")                     \
  V(I32, I32, I32, 0, 0x77, I32_ROTL, "i32.rotl")                       \
  V(I32, I32, I32, 0, 0x78, I32_ROTR, "i32.rotr")                       \
  V(I64, I64, I64, 0, 0x79, I64_CLZ, "i64.clz")                         \
  V(I64, I64, I64, 0, 0x7a, I64_CTZ, "i64.ctz")                         \
  V(I64, I64, I64, 0, 0x7b, I64_POPCNT, "i64.popcnt")                   \
  V(I64, I64, I64, 0, 0x7c, I64_ADD, "i64.add")                         \
  V(I64, I64, I64, 0, 0x7d, I64_SUB, "i64.sub")                         \
  V(I64, I64, I64, 0, 0x7e, I64_MUL, "i64.mul")                         \
  V(I64, I64, I64, 0, 0x7f, I64_DIV_S, "i64.div_s")                     \
  V(I64, I64, I64, 0, 0x80, I64_DIV_U, "i64.div_u")                     \
  V(I64, I64, I64, 0, 0x81, I64_REM_S, "i64.rem_s")                     \
  V(I64, I64, I64, 0, 0x82, I64_REM_U, "i64.rem_u")                     \
  V(I64, I64, I64, 0, 0x83, I64_AND, "i64.and")                         \
  V(I64, I64, I64, 0, 0x84, I64_OR, "i64.or")                           \
  V(I64, I64, I64, 0, 0x85, I64_XOR, "i64.xor")                         \
  V(I64, I64, I64, 0, 0x86, I64_SHL, "i64.shl")                         \
  V(I64, I64, I64, 0, 0x87, I64_SHR_S, "i64.shr_s")                     \
  V(I64, I64, I64, 0, 0x88, I64_SHR_U, "i64.shr_u")                     \
  V(I64, I64, I64, 0, 0x89, I64_ROTL, "i64.rotl")                       \
  V(I64, I64, I64, 0, 0x8a, I64_ROTR, "i64.rotr")                       \
  V(F32, F32, F32, 0, 0x8b, F32_ABS, "f32.abs")                         \
  V(F32, F32, F32, 0, 0x8c, F32_NEG, "f32.neg")                         \
  V(F32, F32, F32, 0, 0x8d, F32_CEIL, "f32.ceil")                       \
  V(F32, F32, F32, 0, 0x8e, F32_FLOOR, "f32.floor")                     \
  V(F32, F32, F32, 0, 0x8f, F32_TRUNC, "f32.trunc")                     \
  V(F32, F32, F32, 0, 0x90, F32_NEAREST, "f32.nearest")                 \
  V(F32, F32, F32, 0, 0x91, F32_SQRT, "f32.sqrt")                       \
  V(F32, F32, F32, 0, 0x92, F32_ADD, "f32.add")                         \
  V(F32, F32, F32, 0, 0x93, F32_SUB, "f32.sub")                         \
  V(F32, F32, F32, 0, 0x94, F32_MUL, "f32.mul")                         \
  V(F32, F32, F32, 0, 0x95, F32_DIV, "f32.div")                         \
  V(F32, F32, F32, 0, 0x96, F32_MIN, "f32.min")                         \
  V(F32, F32, F32, 0, 0x97, F32_MAX, "f32.max")                         \
  V(F32, F32, F32, 0, 0x98, F32_COPYSIGN, "f32.copysign")               \
  V(F64, F64, F64, 0, 0x99, F64_ABS, "f64.abs")                         \
  V(F64, F64, F64, 0, 0x9a, F64_NEG, "f64.neg")                         \
  V(F64, F64, F64, 0, 0x9b, F64_CEIL, "f64.ceil")                       \
  V(F64, F64, F64, 0, 0x9c, F64_FLOOR, "f64.floor")                     \
  V(F64, F64, F64, 0, 0x9d, F64_TRUNC, "f64.trunc")                     \
  V(F64, F64, F64, 0, 0x9e, F64_NEAREST, "f64.nearest")                 \
  V(F64, F64, F64, 0, 0x9f, F64_SQRT, "f64.sqrt")                       \
  V(F64, F64, F64, 0, 0xa0, F64_ADD, "f64.add")                         \
  V(F64, F64, F64, 0, 0xa1, F64_SUB, "f64.sub")                         \
  V(F64, F64, F64, 0, 0xa2, F64_MUL, "f64.mul")                         \
  V(F64, F64, F64, 0, 0xa3, F64_DIV, "f64.div")                         \
  V(F64, F64, F64, 0, 0xa4, F64_MIN, "f64.min")                         \
  V(F64, F64, F64, 0, 0xa5, F64_MAX, "f64.max")                         \
  V(F64, F64, F64, 0, 0xa6, F64_COPYSIGN, "f64.copysign")               \
  V(I32, I64, ___, 0, 0xa7, I32_WRAP_I64, "i32.wrap/i64")               \
  V(I32, F32, ___, 0, 0xa8, I32_TRUNC_S_F32, "i32.trunc_s/f32")         \
  V(I32, F32, ___, 0, 0xa9, I32_TRUNC_U_F32, "i32.trunc_u/f32")         \
  V(I32, F64, ___, 0, 0xaa, I32_TRUNC_S_F64, "i32.trunc_s/f64")         \
  V(I32, F64, ___, 0, 0xab, I32_TRUNC_U_F64, "i32.trunc_u/f64")         \
  V(I64, I32, ___, 0, 0xac, I64_EXTEND_S_I32, "i64.extend_s/i32")       \
  V(I64, I32, ___, 0, 0xad, I64_EXTEND_U_I32, "i64.extend_u/i32")       \
  V(I64, F32, ___, 0, 0xae, I64_TRUNC_S_F32, "i64.trunc_s/f32")         \
  V(I64, F32, ___, 0, 0xaf, I64_TRUNC_U_F32, "i64.trunc_u/f32")         \
  V(I64, F64, ___, 0, 0xb0, I64_TRUNC_S_F64, "i64.trunc_s/f64")         \
  V(I64, F64, ___, 0, 0xb1, I64_TRUNC_U_F64, "i64.trunc_u/f64")         \
  V(F32, I32, ___, 0, 0xb2, F32_CONVERT_S_I32, "f32.convert_s/i32")     \
  V(F32, I32, ___, 0, 0xb3, F32_CONVERT_U_I32, "f32.convert_u/i32")     \
  V(F32, I64, ___, 0, 0xb4, F32_CONVERT_S_I64, "f32.convert_s/i64")     \
  V(F32, I64, ___, 0, 0xb5, F32_CONVERT_U_I64, "f32.convert_u/i64")     \
  V(F32, F64, ___, 0, 0xb6, F32_DEMOTE_F64, "f32.demote/f64")           \
  V(F64, I32, ___, 0, 0xb7, F64_CONVERT_S_I32, "f64.convert_s/i32")     \
  V(F64, I32, ___, 0, 0xb8, F64_CONVERT_U_I32, "f64.convert_u/i32")     \
  V(F64, I64, ___, 0, 0xb9, F64_CONVERT_S_I64, "f64.convert_s/i64")     \
  V(F64, I64, ___, 0, 0xba, F64_CONVERT_U_I64, "f64.convert_u/i64")     \
  V(F64, F32, ___, 0, 0xbb, F64_PROMOTE_F32, "f64.promote/f32")         \
  V(I32, F32, ___, 0, 0xbc, I32_REINTERPRET_F32, "i32.reinterpret/f32") \
  V(I64, F64, ___, 0, 0xbd, I64_REINTERPRET_F64, "i64.reinterpret/f64") \
  V(F32, I32, ___, 0, 0xbe, F32_REINTERPRET_I32, "f32.reinterpret/i32") \
  V(F64, I64, ___, 0, 0xbf, F64_REINTERPRET_I64, "f64.reinterpret/i64")

enum WabtOpcode {
#define V(rtype, type1, type2, mem_size, code, NAME, text) \
  WABT_OPCODE_##NAME = code,
  WABT_FOREACH_OPCODE(V)
#undef V
  WABT_NUM_OPCODES
};

struct WabtOpcodeInfo {
  const char* name;
  WabtType result_type;
  WabtType param1_type;
  WabtType param2_type;
  int memory_size;
};

enum WabtLiteralType {
  WABT_LITERAL_TYPE_INT,
  WABT_LITERAL_TYPE_FLOAT,
  WABT_LITERAL_TYPE_HEXFLOAT,
  WABT_LITERAL_TYPE_INFINITY,
  WABT_LITERAL_TYPE_NAN,
};

struct WabtLiteral {
  WabtLiteralType type;
  WabtStringSlice text;
};

WABT_EXTERN_C_BEGIN
static WABT_INLINE void* wabt_alloc(size_t size) {
  return malloc(size);
}

static WABT_INLINE void* wabt_alloc_zero(size_t size) {
  return calloc(size, 1);
}

static WABT_INLINE void* wabt_realloc(void* p, size_t size) {
  /* Realloc normally frees if size is 0, but we don't want that behavior. */
  if (size == 0)
    return p;
  return realloc(p, size);
}

static WABT_INLINE void wabt_free(void* p) {
  free(p);
}

static WABT_INLINE char* wabt_strndup(const char* s, size_t len) {
  size_t real_len = 0;
  const char* p = s;
  while (real_len < len && *p) {
    p++;
    real_len++;
  }

  char* new_s = static_cast<char*>(wabt_alloc(real_len + 1));
  memcpy(new_s, s, real_len);
  new_s[real_len] = 0;
  return new_s;
}

static WABT_INLINE WabtStringSlice wabt_dup_string_slice(WabtStringSlice str) {
  WabtStringSlice result;
  result.start = wabt_strndup(str.start, str.length);
  result.length = str.length;
  return result;
}

/* return 1 if |alignment| matches the alignment of |opcode|, or if |alignment|
 * is WABT_USE_NATURAL_ALIGNMENT */
bool wabt_is_naturally_aligned(WabtOpcode opcode, uint32_t alignment);

/* if |alignment| is WABT_USE_NATURAL_ALIGNMENT, return the alignment of
 * |opcode|, else return |alignment| */
uint32_t wabt_get_opcode_alignment(WabtOpcode opcode, uint32_t alignment);

WabtStringSlice wabt_empty_string_slice(void);
bool wabt_string_slice_eq_cstr(const WabtStringSlice* s1, const char* s2);
bool wabt_string_slice_startswith(const WabtStringSlice* s1,
                                      const char* s2);
WabtStringSlice wabt_string_slice_from_cstr(const char* string);
bool wabt_string_slice_is_empty(const WabtStringSlice*);
bool wabt_string_slices_are_equal(const WabtStringSlice*,
                                  const WabtStringSlice*);
void wabt_destroy_string_slice(WabtStringSlice*);
WabtResult wabt_read_file(const char* filename,
                          void** out_data,
                          size_t* out_size);

void wabt_default_source_error_callback(const WabtLocation*,
                                        const char* error,
                                        const char* source_line,
                                        size_t source_line_length,
                                        size_t source_line_column_offset,
                                        void* user_data);

void wabt_default_binary_error_callback(uint32_t offset,
                                        const char* error,
                                        void* user_data);

void wabt_init_stdio();

/* opcode info */
extern WabtOpcodeInfo g_wabt_opcode_info[];
void wabt_init_opcode_info(void);

static WABT_INLINE const char* wabt_get_opcode_name(WabtOpcode opcode) {
  assert(opcode < WABT_NUM_OPCODES);
  wabt_init_opcode_info();
  return g_wabt_opcode_info[opcode].name;
}

static WABT_INLINE WabtType wabt_get_opcode_result_type(WabtOpcode opcode) {
  assert(opcode < WABT_NUM_OPCODES);
  wabt_init_opcode_info();
  return g_wabt_opcode_info[opcode].result_type;
}

static WABT_INLINE WabtType wabt_get_opcode_param_type_1(WabtOpcode opcode) {
  assert(opcode < WABT_NUM_OPCODES);
  wabt_init_opcode_info();
  return g_wabt_opcode_info[opcode].param1_type;
}

static WABT_INLINE WabtType wabt_get_opcode_param_type_2(WabtOpcode opcode) {
  assert(opcode < WABT_NUM_OPCODES);
  wabt_init_opcode_info();
  return g_wabt_opcode_info[opcode].param2_type;
}

static WABT_INLINE int wabt_get_opcode_memory_size(WabtOpcode opcode) {
  assert(opcode < WABT_NUM_OPCODES);
  wabt_init_opcode_info();
  return g_wabt_opcode_info[opcode].memory_size;
}

/* external kind */

extern const char* g_wabt_kind_name[];

static WABT_INLINE const char* wabt_get_kind_name(WabtExternalKind kind) {
  assert(kind < WABT_NUM_EXTERNAL_KINDS);
  return g_wabt_kind_name[kind];
}

/* reloc */

extern const char* g_wabt_reloc_type_name[];

static WABT_INLINE const char* wabt_get_reloc_type_name(WabtRelocType reloc) {
  assert(reloc < WABT_NUM_RELOC_TYPES);
  return g_wabt_reloc_type_name[reloc];
}

/* type */

static WABT_INLINE const char* wabt_get_type_name(WabtType type) {
  switch (type) {
    case WABT_TYPE_I32: return "i32";
    case WABT_TYPE_I64: return "i64";
    case WABT_TYPE_F32: return "f32";
    case WABT_TYPE_F64: return "f64";
    case WABT_TYPE_ANYFUNC: return "anyfunc";
    case WABT_TYPE_FUNC: return "func";
    case WABT_TYPE_VOID: return "void";
    case WABT_TYPE_ANY: return "any";
    default: return nullptr;
  }
}

WABT_EXTERN_C_END

#endif /* WABT_COMMON_H_ */