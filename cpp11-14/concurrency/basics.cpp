#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <algorithm>

#include <defines.h>

using utils::type_name;

class Test
{
public:
    Test()
    {
        std::cout << "Test::ctor" << std::endl;
    }
    ~Test()
    {
        std::cout << "Test::dtor" << std::endl;
    }
    // Make Test callable for passing to std::thread
    void operator() () {}
};

class thread_guard
{
private:
    std::thread &m_t;
public:
    explicit thread_guard( std::thread &t )
        : m_t(t)
    {}

    ~thread_guard()
    {
        if ( m_t.joinable() )
            m_t.join();
    }

    thread_guard( const thread_guard & ) = delete;
    thread_guard &operator=( const thread_guard & ) = delete;
};

void challenge1()
{
    // Ctor was called 3 times
    // Default ctor( default with params if we call with int param)
    Test worker;
    // Copy ctor and move ctor( why? )
    // we pass lvalue to thread ctor and thats why worker obj is copied
    // Lets pass it as reference
    std::thread t1{ std::ref(worker) };

    if ( t1.joinable() )
        t1.join();

    // Now we have 2 forwards
    std::thread t2{ std::move(worker) };

    if ( t2.joinable() )
        t2.join();

    // Use lambda
    std::thread t3{ []()
    {
        std::cout << "I am lambda!" << std::endl;
    } };

    if ( t3.joinable() )
        t3.join();
}

void challenge2()
{
    Test worker;
    std::thread t{ std::move(worker) };
    thread_guard g( t );
    throw std::logic_error( "RAII" );
}

// Thread object params
struct Holder
{
    int var;
    Holder( int var_ )
        : var(var_)
    {
        std::cout << "Holder::ctor" << std::endl;
    }

    Holder()
        : Holder(0)
    {}

    ~Holder()
    {
        std::cout << "~Holder::dtor is " << var << std::endl;
    }
};

class A
{
public:
    A()
    {
        std::cout << "A::ctor" << std::endl;
    }
    ~A()
    {
        std::cout << "~A::dtor" << std::endl;
    }

    void thread_method( Holder val, Holder &ref, std::unique_ptr<Holder> &&ptr )
    {
        std::cout << "[thread] val is " << val.var
                  << ", ref is " << ref.var << std::endl;
        std::cout << "[thread] Wait for 5 sec...";
        std::this_thread::sleep_for( std::chrono::seconds(5) );
        std::cout << " Exit!" << std::endl;
        ref.var++;
        (*ptr).var++;
    }
};

void challenge3()
{
    A a;
    Holder val(5);
    Holder ref(10);
    Holder *ptr = new Holder(20);
    std::unique_ptr<Holder> p{ ptr };
    {
        std::thread t { &A::thread_method, &a, val, std::ref(ref), std::move(p) };
        thread_guard g{ t };
    }
    std::cout << "By ref variable is " << ref.var << std::endl;
    std::cout << "Moved pointer variable is " << ptr->var << std::endl;
}

// Syncronization primitives
std::mutex sync_mutex;

class scoped_guard
{
public:
    scoped_guard()
    {
        std::cout << "Thread #" << std::this_thread::get_id() << " started." << std::endl;
    }
    ~scoped_guard()
    {
        std::cout << "Thread #" << std::this_thread::get_id() << " finished." << std::endl;
    }

};

void thread_method()
{
    std::lock_guard<std::mutex> lg( sync_mutex );
    scoped_guard sg;
}

void challenge4()
{
    std::vector<std::thread> v( 10 );

    for (size_t index = 0; index < v.size(); ++index)
        v.at( index ) = std::move( std::thread( thread_method ) );

    std::for_each( v.begin(), v.end(), std::mem_fn( &std::thread::join ) );
}

int main(int argc, char const *argv[])
{
    std::vector<int> v;
    challenge1();

    try
    {
        challenge2();
    }

    catch ( ... ) {}

    challenge3();
    challenge4();
    return 0;
}
