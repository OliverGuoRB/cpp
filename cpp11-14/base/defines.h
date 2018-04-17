#ifndef T_H
#define T_H

#include <iostream>
#include <ctime>
#include <chrono>
#include <ratio>
// Use platform-specific lib
#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <memory>
#include <string>
#include <cstdlib>

namespace utils {

#define UNUSED(x) (void)(x)

/**
    Returns string representation of type
*/
template<class T>
std::string type_name()
{
    typedef typename std::remove_reference<T>::type TR;
    std::unique_ptr<char, void(*)(void*)> own
    (
#ifndef _MSC_VER
        abi::__cxa_demangle(typeid(TR).name(), nullptr,
                            nullptr, nullptr),
#else
        nullptr,
#endif
        std::free
    );
    std::string r = own != nullptr ? own.get() : typeid(TR).name();

    if (std::is_const<TR>::value)
        r += " const";

    if (std::is_volatile<TR>::value)
        r += " volatile";

    if (std::is_lvalue_reference<T>::value)
        r += "&";

    else if (std::is_rvalue_reference<T>::value)
        r += "&&";

    return r;
}

// Capture time
typedef std::chrono::high_resolution_clock::time_point hr_clock_t;

template<typename TType>
class CaptureTime
{
public:

    void Start()
    {
        m_start_time = std::chrono::high_resolution_clock::now();
    }

    int TimePoint(bool reset_start_point = false)
    {
        auto time = std::chrono::high_resolution_clock::now() - m_start_time;

        if (reset_start_point)
        {
            Start();
        }

        return std::chrono::duration_cast<TType>(time).count();
    }
private:

    hr_clock_t m_start_time;
};


} // End of basic_utils name

#endif
