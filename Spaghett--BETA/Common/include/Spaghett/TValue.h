#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace Spaghett
{
    struct UpvalueBox;
    struct SpaghetState;
    struct Table;
    struct Chunk;

    using NativeFunction = void(*)(SpaghetState*, uint8_t argBase, uint8_t nargs, uint8_t retReg);
    using NativeClosure = std::function<void(SpaghetState*, uint8_t, uint8_t, uint8_t)>;

    enum class Type : uint8_t
    {
        NUMBER,
        BOOLEAN,
        NULL_,
        STRING,
        FUNCTION,
        TABLE,
        COROUTINE
    };

    struct TValue
    {
        Type type = Type::NULL_;
        bool isNative = false;
        bool isClosure = false;
        union {
            double num;
            bool b;
            uint16_t str;
            NativeFunction cfunc;
            Chunk* chunk;
            Table* table;
            SpaghetState* co;
        };
        std::vector<std::shared_ptr<UpvalueBox>>* upvalues = nullptr;
        std::shared_ptr<NativeClosure> nativeClosure;

        static TValue Number(double v) { TValue t; t.type = Type::NUMBER;  t.num = v; return t; }
        static TValue Boolean(bool v) { TValue t; t.type = Type::BOOLEAN; t.b = v; return t; }
        static TValue Null() { TValue t; t.type = Type::NULL_;   t.num = 0; return t; }
        static TValue MakeTable();

        static TValue MakeNative(NativeFunction fn)
        {
            TValue t; t.type = Type::FUNCTION; t.isNative = true;  t.cfunc = fn; return t;
        }
        static TValue MakeFunction(Chunk* c)
        {
            TValue t; t.type = Type::FUNCTION; t.isNative = false; t.chunk = c;  return t;
        }

        static TValue MakeCoroutine(SpaghetState* co)
        {
            TValue t;
            t.type = Type::COROUTINE;
            t.co = co;
            return t;
        }

        static TValue MakeNativeClosure(NativeClosure fn)
        {
            TValue t;
            t.type = Type::FUNCTION;
            t.isNative = true;
            t.isClosure = true;
            t.nativeClosure = std::make_shared<NativeClosure>(std::move(fn));
            return t;
        }
    };

    struct Table
    {
        std::unordered_map<std::string, TValue> hash;
        std::vector<TValue> array;
    };

    struct UpvalueBox
    {
        TValue value;
    };

    inline TValue TValue::MakeTable()
    {
        TValue t;
        t.type = Type::TABLE;
        t.table = new Table();
        return t;
    }
}