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

#ifndef WABT_AST_PARSER_H_
#define WABT_AST_PARSER_H_

#include "ast-lexer.h"
#include "common.h"

struct WabtScript;

WABT_EXTERN_C_BEGIN
WabtResult wabt_parse_ast(WabtAstLexer* lexer,
                          struct WabtScript* out_script,
                          WabtSourceErrorHandler*);
WABT_EXTERN_C_END

#endif /* WABT_AST_PARSER_H_ */