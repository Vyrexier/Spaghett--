#include "../include/Spaghett/Sapi.h"
#include <iostream>

SpaghetState* SpaghetNewThread(SpaghetState* state, Spaghett::TValue func)
{
    return Spaghett::NewThread(state, func);
}

void SpaghetResume(SpaghetState* caller, SpaghetState* co)
{
    if (co->status == Spaghett::StateStatus::Finish) return;
    if (co->status == Spaghett::StateStatus::Running) return;

    if (co->status == Spaghett::StateStatus::None)
    {
        Spaghett::CallFrame frame;
        frame.chunk = co->func.chunk;
        frame.pc = 0;
        frame.baseReg = 0;
        frame.retReg = 0;
        frame.outerBase = 0;
        frame.func = co->func;
        co->callStack.push_back(frame);
        co->status = Spaghett::StateStatus::Running;
    }
    else if (co->status == Spaghett::StateStatus::Yield)
        co->status = Spaghett::StateStatus::Running;

    co->caller = caller;

    if (setjmp(co->callerJmp) == 0)
    {
        Spaghett::VM::Execute(*co);
        co->status = Spaghett::StateStatus::Finish;
    }
}

void SpaghetYield(SpaghetState* state)
{
    if (!state->caller) return;
    state->status = Spaghett::StateStatus::Yield;
    longjmp(state->callerJmp, 1);
}

bool SpaghetIsCoroutine(SpaghetState* state)
{
    return state->caller != nullptr;
}

bool SpaghetThreadFinished(SpaghetState* co)
{
    return co->status == Spaghett::StateStatus::Finish;
}

bool SpaghetThreadSuspended(SpaghetState* co)
{
    return co->status == Spaghett::StateStatus::Yield ||
        co->status == Spaghett::StateStatus::None;
}

void SpaghetSpawn(SpaghetState* state, Spaghett::TValue func)
{
    if (!state->scheduler) return;
    SpaghetState* co = Spaghett::NewThread(state, func);
    co->scheduler = state->scheduler;
    state->scheduler->Spawn(co);
}

void SpaghetSetGlobal(SpaghetState* state, const char* name, Spaghett::TValue value)
{
    state->G->globals[name] = value;
}

Spaghett::TValue SpaghetGetGlobal(SpaghetState* state, const char* name)
{
    auto it = state->G->globals.find(name);
    if (it != state->G->globals.end())
        return it->second;
    return Spaghett::TValue::Null();
}

bool SpaghetHasError(SpaghetState* state)
{
    return state->status == Spaghett::StateStatus::Error;
}

const char* SpaghetGetError(SpaghetState* state)
{
    return state->error.c_str();
}

void SpaghetClearError(SpaghetState* state)
{
    state->error.clear();
    state->status = Spaghett::StateStatus::None;
}

bool SpaghetIsRunning(SpaghetState* state)
{
    return state->status == Spaghett::StateStatus::Running;
}

bool SpaghetIsFinished(SpaghetState* state)
{
    return state->status == Spaghett::StateStatus::Finish;
}

SpaghetState* SpaghetOpen()
{
    auto* state = Spaghett::OpenSpaghettState();

    Spaghett::Scheduler* scheduler = new Spaghett::Scheduler();
    state->scheduler = scheduler;

    Spaghett::OpenStdLib(state);
    return state;
}

void SpaghetClose(SpaghetState* state)
{
    delete state->scheduler;
    Spaghett::CloseSpaghettState(state);
}

SpaghetChunk SpaghetCompile(SpaghetState* state, const char* source)
{
    Spaghett::Lexer lexer(source);
    lexer.Tokenize();

    auto tokens = lexer.GetTokens();
    Spaghett::Parser parser(std::move(tokens));
    parser.Parse();

    Spaghett::SemanticAnalyzer analyzer;
    analyzer.Analyze(parser.GetStatements());
    for (auto& err : analyzer.GetErrors())
        std::cout << err << "\n";

    Spaghett::BytecodeGenerator gen;
    return gen.Generate(parser.GetStatements(), state->G);
}

void SpaghetLoad(SpaghetState* state, const SpaghetChunk& chunk)
{
    Spaghett::VM vm;
    vm.Load(*state, chunk);
}

void SpaghetRun(SpaghetState* state)
{
    Spaghett::VM vm;
    vm.Call(*state);
}

void SpaghetExec(SpaghetState* state, const char* source)
{
    auto chunk = SpaghetCompile(state, source);

    SpaghetLoad(state, chunk);
    SpaghetRun(state);
}

void SpaghetDebugTokens(const char* source)
{
    Spaghett::Lexer lexer(source);
    lexer.Tokenize();
    lexer.ViewTokens();
}

void SpaghetDebugAST(const char* source)
{
    Spaghett::Lexer lexer(source);
    lexer.Tokenize();
    auto tokens = lexer.GetTokens();
    Spaghett::Parser parser(std::move(tokens));
    parser.Parse();
    parser.ViewAst();
}

void SpaghetDebugBytecode(SpaghetState* state, const char* source)
{
    auto chunk = SpaghetCompile(state, source);
    chunk.Dump();
}

void SpaghetDebugAll(SpaghetState* state, const char* source)
{
    SpaghetDebugTokens(source);
    SpaghetDebugAST(source);
    SpaghetDebugBytecode(state, source);
}