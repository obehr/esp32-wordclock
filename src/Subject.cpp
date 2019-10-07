
#include "Subject.hpp"

Subject::Subject() : obs(0)
{

}

void Subject::attach(Observer& o)
{
    // Store the pointer
    this->obs = &o;
}

void Subject::notify()
{
    if (this->obs)
    {
        this->obs->notify();
    }
}
