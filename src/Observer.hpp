#ifndef __OBSERVER_HPP__
#define __OBSERVER_HPP__

// Abstract observer class
class Observer
{
public:
    /**
     * Virtual destructor.
     */
    virtual ~Observer() {};

    /**
     * Notify observer.
     */
    virtual void notify() = 0;
};

#endif /* __OBSERVER_HPP__ */
