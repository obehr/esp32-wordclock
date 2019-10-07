#ifndef __SUBJECT_HPP__
#define __SUBJECT_HPP__

#include "Observer.hpp"

// Subject is observed by Observer
class Subject
{
public:
    Subject();

    /**
     * Attach observer to subject.
     *
     * One observer is supported. Existing observer will get replaced.
     */
    void attach(Observer& o);

    /**
     * Notify attached observers.
     *
     * Do nothing, if no observer is attached.
     */
    void notify();

private:
    /**
     * Pointer to observer.
     *
     * A pointer is needed, because no observer can be attachend. The pointer
     * is null then.
     */
    Observer* obs;
};

#endif /* __SUBJECT_HPP__ */
