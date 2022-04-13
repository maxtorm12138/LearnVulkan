// boost
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

// std
#include <exception>

#include "lvk_engine.hpp"

void init_log()
{
    boost::log::core::get()->set_filter(boost::log::trivial::severity > boost::log::trivial::debug);
}

int main(int, char* argv[])
{
    try
    {
        init_log();
        lvk::Engine engine;
        engine.Run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}