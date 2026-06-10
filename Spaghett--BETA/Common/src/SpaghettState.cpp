// SpaghetState.cpp
#include "../include/Spaghett/SpaghetState.h"

namespace Spaghett
{
    SpaghetState* OpenSpaghettState()
    {
        GlobalState* G = new GlobalState();
        SpaghetState* state = new SpaghetState();
        state->G = G;
        state->registers.fill(TValue::Null());
        return state;
    }

    SpaghetState* NewThread(SpaghetState* parent, TValue func)
    {
        SpaghetState* co = new SpaghetState();
        co->G = parent->G;
        co->func = func;
        co->status = StateStatus::None;
        co->registers.fill(TValue::Null());
        return co;
    }

    void CloseSpaghettState(SpaghetState* state)
    {
        delete state->G;
        delete state;
    }
}