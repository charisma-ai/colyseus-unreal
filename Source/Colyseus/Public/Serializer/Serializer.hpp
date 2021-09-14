#ifndef Serializer_hpp
#define Serializer_hpp

#include "schema.h"

template <typename S>
class Serializer
{
public:
//    Serializer();
//    virtual ~Serializer();

    virtual S* getState() = 0;
    virtual void setState(unsigned const char* bytes, int offset, int length) = 0;
    virtual void patch(unsigned const char* bytes, int offset, int length) = 0;
    virtual void teardown() = 0;
    virtual void handshake(unsigned const char* bytes, int offset) = 0;
};

#endif /* Serializer_hpp */
