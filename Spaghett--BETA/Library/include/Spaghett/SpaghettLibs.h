#pragma once
#include "../../../Common/include/Spaghett/SpaghetState.h"

#include <chrono>
#include <ctime>

namespace Spaghett
{
    void print(SpaghetState* state, uint8_t funcReg, uint8_t nargs, uint8_t retReg); // print in same line
    void printl(SpaghetState* state, uint8_t funcReg, uint8_t nargs, uint8_t retReg); //print with new line

    void tostring(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void tonumber(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);

    void os_time(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void os_clock(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void os_date(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);

    void io_read(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void io_write(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);

    void coroutine_spawn(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void coroutine_wait(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void coroutine_create(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void coroutine_resume(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void coroutine_yield(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void coroutine_status(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);

    Table* CreateTable(SpaghetState* state, const char* tableName);
    void RegisterTableFunc(Table* table, const char* name, NativeFunction fn);

    void pcall(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    void error(SpaghetState* state, uint8_t argBase, uint8_t nargs, uint8_t retReg);

    void RegisterFunction(SpaghetState* state, const char* name, NativeFunction fn);
    void OpenStdLib(SpaghetState* state);
}