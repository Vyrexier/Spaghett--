#pragma once
#include "../../../Common/include/Spaghett/SpaghetState.h"
#include "../../../Common/include/Spaghett/Chunk.h"
#include <chrono>
#include <queue>

namespace Spaghett
{

    inline double GetTime()
    {
        using namespace std::chrono;
        auto now = high_resolution_clock::now().time_since_epoch();
        return duration<double>(now).count();
    }

    class VM
    {
    public:
        void Load(SpaghetState& state, const Chunk& chunk);
        void Call(SpaghetState& state, int nargs = 0, int nresults = 0);
        static void Execute(SpaghetState& state);
    private:
    };
}