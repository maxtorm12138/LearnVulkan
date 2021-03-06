#ifndef _LVK_ENGINE_H
#define _LVK_ENGINE_H

// boost
#include <boost/noncopyable.hpp>

// std
#include <memory>

namespace lvk
{
namespace detail
{

class EngineImpl;
struct EngineImplDeleter
{
    void operator()(EngineImpl *);
};

}

class Engine : public boost::noncopyable
{
public:
    Engine();

public:
    void Run();

private:
    std::unique_ptr<detail::EngineImpl, detail::EngineImplDeleter> impl_;
};

};
#endif