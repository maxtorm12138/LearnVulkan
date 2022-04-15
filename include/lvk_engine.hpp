#pragma once

// boost
#include <boost/noncopyable.hpp>

namespace lvk
{
namespace detail
{
class EngineImpl;
}

class Engine : public boost::noncopyable
{
public:
    Engine();
    ~Engine();

    void Run();
private:
    detail::EngineImpl *impl_;
};

};