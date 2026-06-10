#pragma once

#include "../../../Syntax/include/Spaghett/Lexer.h"
#include "../../../Syntax/include/Spaghett/Parser.h"
#include "../../../Analyzer/include/Spaghett/SemanticAnalyzer.h"
#include "../../../ByteCode/include/Spaghett/BytecodeGenerator.h"
#include "../../../Common/include/Spaghett/SpaghetState.h"
#include "../../../VM/include/Spaghett/VirtualMachine.h"
#include "../../../Library/include/Spaghett/SpaghettLibs.h"

using SpaghetState = Spaghett::SpaghetState;
using SpaghetChunk = Spaghett::Chunk;

// lifecycle
SpaghetState* SpaghetOpen();
void          SpaghetClose(SpaghetState* state);

// compile & run
SpaghetChunk SpaghetCompile(SpaghetState* state, const char* source);
void          SpaghetLoad(SpaghetState* state, const SpaghetChunk& chunk);
void          SpaghetRun(SpaghetState* state);
void          SpaghetExec(SpaghetState* state, const char* source);

// globals
void          SpaghetSetGlobal(SpaghetState* state, const char* name, Spaghett::TValue value);
Spaghett::TValue SpaghetGetGlobal(SpaghetState* state, const char* name);

// error
bool          SpaghetHasError(SpaghetState* state);
const char* SpaghetGetError(SpaghetState* state);
void          SpaghetClearError(SpaghetState* state);

// status
bool          SpaghetIsRunning(SpaghetState* state);
bool          SpaghetIsFinished(SpaghetState* state);

// debug
void          SpaghetDebugTokens(const char* source);
void          SpaghetDebugAST(const char* source);
void			  SpaghetDebugBytecode(SpaghetState* state, const char* source);
void			  SpaghetDebugAll(SpaghetState* state, const char* source);

SpaghetState* SpaghetNewThread(SpaghetState* state, Spaghett::TValue func);
void          SpaghetResume(SpaghetState* caller, SpaghetState* co);
void          SpaghetYield(SpaghetState* state);
bool          SpaghetIsCoroutine(SpaghetState* state);
bool          SpaghetThreadFinished(SpaghetState* co);
bool          SpaghetThreadSuspended(SpaghetState* co);

// scheduler
void          SpaghetSpawn(SpaghetState* state, Spaghett::TValue func);