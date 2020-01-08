#ifndef KEYVARIANT_H
#define KEYVARIANT_H

#include <boost/variant.hpp>
// std cont
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <iostream>

template<class ... Args>
class KeyVariant
{
public:
    typedef boost::variant<Args...> ValueType;

public:
    template<class T>
    void set(const char *name, T value) {
        m_variants[name] = value;
    }

    void set(const char *name, ValueType value) {
        m_variants[name] = value;
    }

    template<class T>
    T get(const char *name) {
        if (nullptr == name)
            return T();

        auto iter = m_variants.find(name);
        if (m_variants.end() == iter)
            return T();

        if (typeid(T) != iter->second.type())
            return T();

        return boost::get<T>(iter->second);
    }

    void remove(const char *name) {
        auto iter = m_variants.find(name);
        if (m_variants.end() != iter)
        {
            m_variants.erase(iter);
        }
    }

    template<class T>
    bool has(const char *name) {
        auto iter = m_variants.find(name);
        return m_variants.end() != iter;
    }

    template<class T>
    bool is(const char *name) {
        auto iter = m_variants.find(name);
        if (m_variants.end() == iter)
            return false;

        if (typeid(T) != iter->second.type())
            return false;

        return true;
    }

private:
    std::unordered_map<std::string, ValueType> m_variants;
};

#endif // KEYVARIANT_H
