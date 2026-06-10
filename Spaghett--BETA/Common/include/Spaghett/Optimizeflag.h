#pragma once

struct OptimizeConfig
{
    bool ConstantFolding = true;
    bool ConstantPropagation = true;

    bool DeadCodeElimination = true;
    bool OptimizeJumps = true;
};