#ifndef SchemaSerializer_hpp
#define SchemaSerializer_hpp

#include "schema.h"
#include "Serializer.hpp"

template <typename S>
class SchemaSerializer : public Serializer<S>
{
public:
    SchemaSerializer() {
        state = new S();
        it = new colyseus::schema::Iterator();
    }
    ~SchemaSerializer() {
        delete state;
        delete it;
    }

    colyseus::schema::Iterator *it;
    S* state;
    S* getState() { return state; };

    void setState(unsigned const char* bytes, int offset, int length) {
        it->offset = offset;
        ((colyseus::schema::Schema*)state)->decode(bytes, length, it);
    }

    void patch(unsigned const char* bytes, int offset, int length) {
        it->offset = offset;
        ((colyseus::schema::Schema*)state)->decode(bytes, length, it);
    }

    void handshake(unsigned const char* bytes, int offset) {
        // TODO: validate incoming schema with Reflection.
    }

    void teardown() {
    }
};

#endif /* SchemaSerializer_hpp */
