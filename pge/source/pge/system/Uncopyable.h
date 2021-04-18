#pragma once

// Inherit from this class to make the class uncopyable
namespace pge {
    class Uncopyable {
    protected:
        Uncopyable() {}
        virtual ~Uncopyable() {}
    private:
        Uncopyable(const Uncopyable &);
        Uncopyable &operator=(const Uncopyable &);
    };
}