#pragma once
#include <unordered_set>
#include <string>

namespace Spaghett
{
    class BuiltInRegistry
    {
    public:
        static BuiltInRegistry& Get()
        {
            static BuiltInRegistry instance;
            return instance;
        }

        const std::unordered_set<std::string>& GetAll() const 
        {
            return m_names; 
        }

        void Register(const std::string& name)
        {
            m_names.insert(name);
        }

        bool Contains(const std::string& name) const
        {
            return m_names.count(name) > 0;
        }

    private:
        std::unordered_set<std::string> m_names;
    };
}