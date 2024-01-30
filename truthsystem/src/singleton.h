#ifndef SINGLETON_H
#define SINGLETON_H


template <typename T>
class  Singleton
{
public:
    Singleton() = default;
    static T * instance()
    {
        static T object;
        return & object;
    }
};

#endif // SINGLETON_H
