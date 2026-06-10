#include "../include/Spaghett/SpaghettLibs.h"
#include "../../Analyzer/include/Spaghett/BuiltInRegistry.h"

namespace Spaghett
{
	Table* CreateTable(SpaghetState* state, const char* tableName)
	{
		TValue tbl = TValue::MakeTable();
		state->G->globals[tableName] = tbl;
		return state->G->globals[tableName].table;
	}

	void RegisterTableFunc(Table* table, const char* name, NativeFunction fn)
	{
		table->hash[name] = TValue::MakeNative(fn);
	}

	void RegisterFunction(SpaghetState* state, const char* name, NativeFunction fn)
	{
		TValue tv;
		tv = TValue::MakeNative(fn);
		state->G->globals[name] = tv;

		BuiltInRegistry::Get().Register(name);
	}

	void OpenStdLib(SpaghetState* state)
	{
		RegisterFunction(state, "print", print);
		RegisterFunction(state, "printl", printl);
		RegisterFunction(state, "tostring", tostring);
		RegisterFunction(state, "tonumber", tonumber);
		RegisterFunction(state, "pcall", pcall);
		RegisterFunction(state, "error", error);

		Table* coroutine = CreateTable(state, "coroutine");
		RegisterTableFunc(coroutine, "create", coroutine_create);
		RegisterTableFunc(coroutine, "resume", coroutine_resume);
		RegisterTableFunc(coroutine, "yield", coroutine_yield);
		RegisterTableFunc(coroutine, "status", coroutine_status);
		RegisterTableFunc(coroutine, "spawn", coroutine_spawn);
		RegisterTableFunc(coroutine, "wait", coroutine_wait);

		Table* os = CreateTable(state, "os");
		RegisterTableFunc(os, "time", os_time);
		RegisterTableFunc(os, "clock", os_clock);
		RegisterTableFunc(os, "date", os_date);

		Table* io = CreateTable(state, "io");
		RegisterTableFunc(io, "read", io_read);
		RegisterTableFunc(io, "write", io_write);
	}
}