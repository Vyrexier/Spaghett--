#include "Api/include/Spaghett/Sapi.h"
#include <crtdbg.h>

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    const char* source = R"(
-- Test 1: Basic types
local t1 = os.clock()

local n = 42
local s = "hello"
local b = true
local null_val = null
printl(n)
printl(s)
printl(b)

-- Test 2: Arithmetic
local a = 10
local c = 3
printl(a + c)
printl(a - c)
printl(a * c)
printl(a / c)
printl(a % c)

-- Test 4: If/else
local x = 10
if x > 5 then
    print("x is big")
elseif x == 5 then
    print("x is five")
else
    print("x is small")
end

-- Test 5: While loop
local i = 1
while i <= 3 do
    print(i)
    i = i + 1
end

-- Test 6: For numeric
for i = 1, 5, 1 do
    print(i)
end

-- Test 7: Function
func add(a, b)
    return a + b
end
print(add(3, 4))

-- Test 8: Closure
func makeCounter()
    local count = 0
    return func()
        count = count + 1
        return count
    end
end
local counter = makeCounter()
printl(counter())
printl(counter())
printl(counter())

-- Test 9: Table
local t = {x = 1, y = 2}
printl(t.x)
printl(t.y)
t.z = 3
printl(t.z)

)";

auto* state = SpaghetOpen();

auto chunk = SpaghetCompile(state, source);

SpaghetDebugTokens(source);
SpaghetDebugAST(source);
chunk.Dump(&state->G->strings);

SpaghetLoad(state, chunk);
SpaghetRun(state);

SpaghetClose(state);
}