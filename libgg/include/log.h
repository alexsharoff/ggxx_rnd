#pragma once

#include <fstream>
#include <iostream>


#define LIBGG_LOG_PREFIX '[' << time(0) << "] " << __FUNCTION__ << ": "

#ifdef LIBGG_LOG_CONSOLE
#define LIBGG_LOG() std::cout << LIBGG_LOG_PREFIX
#elif defined(LIBGG_LOG_FILE)
namespace
{
struct file_log_t
{
    file_log_t()
    {
        const auto fname = "libgg." + std::to_string(::GetCurrentProcessId()) + ".log";
        m_ofs.open(fname.c_str());
    }
    ~file_log_t()
    {
        m_ofs.close();
    }
    std::ostream& get()
    {
        return m_ofs;
    }
private:
    std::ofstream m_ofs;
} g_log;
}
#define LIBGG_LOG() g_log.get() << LIBGG_LOG_PREFIX
#else
namespace
{
struct dev_null_t
{
    template<typename T>
    dev_null_t& operator<<(const T&) { return *this; }
    template<typename T, size_t N>
    dev_null_t& operator<<(const T(&)[N]) { return *this; }
    // for std::endl et al.
    dev_null_t& operator<<(std::ostream&(*)(std::ostream&)) { return *this; }
};
}
#define LIBGG_LOG() dev_null_t()
#endif
