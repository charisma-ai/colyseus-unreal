/**
 * @colyseus/schema decoder for C/C++
 * Do not modify this file unless you know exactly what you're doing.
 *
 * This file is part of Colyseus: https://github.com/colyseus/colyseus
 */
#pragma once

#include <iostream>
#include <stdint.h>

#include <cstring>
#include <functional>
#include <vector>
#include <string>
#include <map>

#include <typeinfo>
#include <typeindex>

#include "ordered_map.h"

namespace colyseus
{
namespace schema
{

using varint_t = float; // "number"
using string = std::string;
using float32_t = float;
using float64_t = double;

enum class SPEC : unsigned char
{
    END_OF_STRUCTURE = 0xc1, // (msgpack spec: never used)
    NIL = 0xc0,
    INDEX_CHANGE = 0xd4,
};

struct Iterator
{
    size_t offset = 0;
};

// template <typename T>
struct DataChange
{
    string field;
    // T value;
    // T previousValue;
};

// inline bool IsLittleEndian();

 inline string decodeString(unsigned const char bytes[], Iterator *it);
 inline int8_t decodeInt8(unsigned const char bytes[], Iterator *it);
 inline uint8_t decodeUint8(unsigned const char bytes[], Iterator *it);
 inline int16_t decodeInt16(unsigned const char bytes[], Iterator *it);
 inline uint16_t decodeUint16(unsigned const char bytes[], Iterator *it);
 inline int32_t decodeInt32(unsigned const char bytes[], Iterator *it);
 inline uint32_t decodeUint32(unsigned const char bytes[], Iterator *it);
 inline int64_t decodeInt64(unsigned const char bytes[], Iterator *it);
 inline uint64_t decodeUint64(unsigned const char bytes[], Iterator *it);
 inline float32_t decodeFloat32(unsigned const char bytes[], Iterator *it);
 inline float64_t decodeFloat64(unsigned const char bytes[], Iterator *it);
 inline varint_t decodeNumber(unsigned const char bytes[], Iterator *it);
 inline bool decodeBoolean(unsigned const char bytes[], Iterator *it);
 inline bool numberCheck(unsigned const char bytes[], Iterator *it);
 inline bool arrayCheck (unsigned const char bytes[], Iterator *it);
 inline bool nilCheck(unsigned const char bytes[], Iterator *it);
 inline bool indexChangeCheck(unsigned const char bytes[], Iterator *it);

inline bool IsLittleEndian()
{
    int i = 1;
    return (int)*((unsigned char *)&i) == 1;
}

inline string decodeString(unsigned const char bytes[], Iterator *it)
{
    unsigned char prefix = bytes[it->offset++];
    unsigned int length = 0;

    if (prefix < 0xc0)
    {
        length = prefix & 0x1f;
    }
    else if (prefix == 0xd9)
    {
        length = decodeUint8(bytes, it);
    }
    else if (prefix == 0xda)
    {
        length = decodeUint16(bytes, it);
    }
    else if (prefix == 0xdb)
    {
        length = decodeUint32(bytes, it);
    }

    char * str = new char[length + 1];
    std::memcpy(str, bytes + it->offset, length);
    str[length] = '\0'; // string termination
    it->offset += length;

    string value(str);
    delete[] str;
    return value;
}

inline int8_t decodeInt8(unsigned const char bytes[], Iterator *it)
{
    return (int8_t)(bytes[it->offset++] << 24 >> 24);
}

inline uint8_t decodeUint8(unsigned const char bytes[], Iterator *it)
{
    return (uint8_t)bytes[it->offset++];
}

inline int16_t decodeInt16(unsigned const char bytes[], Iterator *it)
{
    int16_t value = *(int16_t *)&bytes[it->offset];
    it->offset += 2;
    return value;
}

inline uint16_t decodeUint16(unsigned const char bytes[], Iterator *it)
{
    uint16_t value = *(uint16_t *)&bytes[it->offset];
    it->offset += 2;
    return value;
}

inline int32_t decodeInt32(unsigned const char bytes[], Iterator *it)
{
    int32_t value = *(int32_t *)&bytes[it->offset];
    it->offset += 4;
    return value;
}

inline uint32_t decodeUint32(unsigned const char bytes[], Iterator *it)
{
    uint32_t value = *(uint32_t *)&bytes[it->offset];
    it->offset += 4;
    return value;
}

inline int64_t decodeInt64(unsigned const char bytes[], Iterator *it)
{
    int64_t value = *(int64_t *)&bytes[it->offset];
    it->offset += 8;
    return value;
}

inline uint64_t decodeUint64(unsigned const char bytes[], Iterator *it)
{
    uint64_t value = *(uint64_t *)&bytes[it->offset];
    it->offset += 8;
    return value;
}

inline float32_t decodeFloat32(unsigned const char bytes[], Iterator *it)
{
    float32_t value = *(float32_t *)&bytes[it->offset];
    it->offset += 4;
    return value;
}

inline float64_t decodeFloat64(unsigned const char bytes[], Iterator *it)
{
    float64_t value = *(float64_t *)&bytes[it->offset];
    it->offset += 8;
    return value;
}

inline varint_t decodeNumber(unsigned const char bytes[], Iterator *it)
{
    auto prefix = bytes[it->offset++];
#ifdef COLYSEUS_DEBUG
    std::cout << "decodeNumber, prefix => " << ((int)prefix) << std::endl;
#endif

    if (prefix < 0x80)
    {
        // positive fixint
        return (varint_t)prefix;
    }
    else if (prefix == 0xca)
    {
        // float 32
        return decodeFloat32(bytes, it);
    }
    else if (prefix == 0xcb)
    {
        // float 64
        return (varint_t) decodeFloat64(bytes, it);
    }
    else if (prefix == 0xcc)
    {
        // uint 8
        return (varint_t)decodeUint8(bytes, it);
    }
    else if (prefix == 0xcd)
    {
        // uint 16
        return (varint_t) decodeUint16(bytes, it);
    }
    else if (prefix == 0xce)
    {
        // uint 32
        return (varint_t) decodeUint32(bytes, it);
    }
    else if (prefix == 0xcf)
    {
        // uint 64
        return (varint_t) decodeUint64(bytes, it);
    }
    else if (prefix == 0xd0)
    {
        // int 8
        return (varint_t) decodeInt8(bytes, it);
    }
    else if (prefix == 0xd1)
    {
        // int 16
        return (varint_t) decodeInt16(bytes, it);
    }
    else if (prefix == 0xd2)
    {
        // int 32
        return (varint_t) decodeInt32(bytes, it);
    }
    else if (prefix == 0xd3)
    {
        // int 64
        return (varint_t) decodeInt64(bytes, it);
    }
    else if (prefix > 0xdf)
    {
        // negative fixint
        return (varint_t) ((0xff - prefix + 1) * -1);
    }
    else
    {
        return 0;
    }
}

inline bool decodeBoolean(unsigned const char bytes[], Iterator *it)
{
    return decodeUint8(bytes, it) > 0;
}

inline bool numberCheck(unsigned const char bytes[], Iterator *it)
{
    auto prefix = bytes[it->offset];
    return (prefix < 0x80 || (prefix >= 0xca && prefix <= 0xd3));
}

inline bool arrayCheck (unsigned const char bytes[], Iterator *it) {
  return bytes[it->offset] < 0xa0;
}

inline bool nilCheck(unsigned const char bytes[], Iterator *it) {
  return bytes[it->offset] == (unsigned char) SPEC::NIL;
}

inline bool indexChangeCheck(unsigned const char bytes[], Iterator *it) {
  return bytes[it->offset] == (unsigned char) SPEC::INDEX_CHANGE;
}

template <typename T>
class ArraySchema
{
  public:
    ArraySchema() {}

    std::vector<T> items;

    std::function<void(T, int)> onAdd;
    std::function<void(T, int)> onChange;
    std::function<void(T, int)> onRemove;

    inline T &operator[](const int &index) { return items[index]; }
    inline T at(const int &index) { return items[index]; }

    inline ArraySchema<T> clone()
    {
        ArraySchema<T> cloned;
        cloned.items = this->items;
        cloned.onAdd = this->onAdd;
        cloned.onRemove = this->onRemove;
        cloned.onChange = this->onChange;
        return cloned;
    }

    inline void setAt(int index, const T& value) {
        if (items.size() == index) {
            items.push_back(value);
        }
        else {
            items[index] = value;
        }
    }

    inline bool has(int index)
    {
        return items.size() > index;
    }

    inline int size()
    {
        return items.size();
    }
};

template <typename T>
class MapSchema
{
  public:
    MapSchema() {}

    tsl::ordered_map<string, T> items;

    std::function<void(T, const string &)> onAdd;
    std::function<void(T, const string &)> onChange;
    std::function<void(T, const string &)> onRemove;

    inline T &operator[](const char index[])
    {
        return items[index];
    }

    inline MapSchema<T> clone()
    {
        MapSchema<T> cloned;
        cloned.items = this->items;
        cloned.onAdd = this->onAdd;
        cloned.onRemove = this->onRemove;
        cloned.onChange = this->onChange;
        return cloned;
    }

    inline std::vector<string> keys()
    {
#ifdef COLYSEUS_DEBUG
        std::cout << "GETTING PREVIOUS KEYS" << std::endl;
#endif
        std::vector<string> keys;

        for (auto kv : this->items)
        {
            keys.push_back(kv.first);
        }

#ifdef COLYSEUS_DEBUG
        for (tsl::ordered_map<string, char *>::iterator it = this->items.begin(); it != this->items.end(); ++it)
        {
            std::cout << "MAP, PREVIOUS KEY => " << it->first << std::endl;
        }
#endif

#ifdef COLYSEUS_DEBUG
        std::cout << "END GETTING PREVIOUS KEYS" << std::endl;
#endif

        return keys;
    }

    inline T at(const string &key)
    {
        return items.at(key);
    }

    inline bool has(const string &field)
    {
        return items.find(field) != items.end();
    }

    inline void insert(const string &field, T value)
    {
        items.insert(std::pair<string, T>(field, value));
    }

    inline int size()
    {
        return items.size();
    }
};

class Schema
{
  public:
    std::function<void(std::vector<DataChange>)> onChange;
    std::function<void()> onRemove;

    Schema() {}

    template <typename T>
    inline void decodeArrayPrimitive(ArraySchema<T> &array, int index, unsigned const char bytes[], Iterator *it,
                              T (*decoder)(unsigned const char bytes[], Iterator *it) ) {
        array.setAt(index, decoder(bytes, it));
    }

    inline void decode(unsigned const char bytes[], int totalBytes, Iterator *it = nullptr) //new Iterator())
    {
        bool doesOwnIterator = it == nullptr;
        if (doesOwnIterator) it = new Iterator();

        std::vector<DataChange> changes;

        while (it->offset < totalBytes)
        {
            bool isNil = nilCheck(bytes, it);
            if (isNil) { it->offset++; }

            unsigned char index = (unsigned char) bytes[it->offset++];
#ifdef COLYSEUS_DEBUG
            std::cout << "INDEX: " << ((int)index) << std::endl;
#endif

            if (index == (unsigned char) SPEC::END_OF_STRUCTURE)
            {
                break;
            }

            // TODO: for backwards compatibility, check existance of field before calling .at()
            string field = this->_indexes.at(index);
            string type = this->_types.at(index);

            bool hasChange = false;

            if (isNil)
            {
                hasChange = true;
            }
            else if (type == "ref")
            {
                Schema* value = this->getRef(field);

                if (value == nullptr) {
                    auto childType = this->_childSchemaTypes.at(index);
                    value = this->createInstance(childType);
                }

                value->decode(bytes, totalBytes, it);
                hasChange = true;

            }
            else if (type == "array")
            {
                ArraySchema<char *> *valueRef = this->getArray(field);
                ArraySchema<char *> *value = valueRef; // valueRef->clone();

                int newLength = decodeNumber(bytes, it);
                int numChanges = decodeNumber(bytes, it);

                bool hasRemoval = (value->items.size() > newLength);
                hasChange = (numChanges > 0) || hasRemoval;

                bool isSchemaType = this->_childSchemaTypes.find(index) != this->_childSchemaTypes.end();

                // FIXME: this may not be reliable. possibly need to encode this variable during
                // serializagion
                bool hasIndexChange = false;

                // ensure current array has the same length as encoded one
                if (hasRemoval) {
                    for (int i = newLength; i < value->items.size(); i++)
                    {
                        if (isSchemaType && ((Schema*)value->items[i])->onRemove)
                        {
                            ((Schema *)value->items[i])->onRemove();
                        }
                        if (valueRef->onRemove)
                        {
                            valueRef->onRemove(value->items[i], i);
                        }
                    }
                    value->items.resize(newLength);
                }

                for (int i = 0; i < numChanges; i++)
                {
                    int newIndex = (int) decodeNumber(bytes, it);

                    int indexChangedFrom = -1; // index change check
                    if (indexChangeCheck(bytes, it)) {
                        /*
                        it->offset++;
                        indexChangedFrom = (int) decodeNumber(bytes, it);
                        hasIndexChange = true;*/

                        decodeUint8(bytes, it);
                        indexChangedFrom = (int) decodeNumber(bytes, it);
                        hasIndexChange = true;

                    }

                    bool isNew = (!hasIndexChange && !value->has(newIndex)) || (hasIndexChange && indexChangedFrom == -1);

                    if (isSchemaType)
                    {
                        char* item;

                        if (isNew)
                        {
                            item = (char *)this->createInstance(this->_childSchemaTypes.at(index));
                        }
                        else if (indexChangedFrom != -1)
                        {
                            item = (char*) valueRef->at(indexChangedFrom);
                        }
                        else
                        {
                            item = (char *) valueRef->at(newIndex);
                        }

                        if (!item)
                        {
                            item = (char *)this->createInstance(this->_childSchemaTypes.at(index));
                            isNew = true;
                        }

                        ((Schema*) item)->decode(bytes, totalBytes, it);
                        value->setAt(newIndex, item);
                    }
                    else
                    {
                        // FIXME: this is ugly and repetitive
                        string primitiveType = this->_childPrimitiveTypes.at(index);

                        if (primitiveType == "string")       { ((ArraySchema<string> *)value)->setAt(newIndex, decodeString(bytes, it)); }
                        else if (primitiveType == "number")  { ((ArraySchema<varint_t> *)value)->setAt(newIndex, decodeNumber(bytes, it)); }
                        else if (primitiveType == "boolean") { ((ArraySchema<bool> *)value)->setAt(newIndex, decodeBoolean(bytes, it)); }
                        else if (primitiveType == "int8")    { ((ArraySchema<int8_t> *)value)->setAt(newIndex, decodeInt8(bytes, it)); }
                        else if (primitiveType == "uint8")   { ((ArraySchema<uint8_t> *)value)->setAt(newIndex, decodeUint8(bytes, it)); }
                        else if (primitiveType == "int16")   { ((ArraySchema<int16_t> *)value)->setAt(newIndex, decodeInt16(bytes, it)); }
                        else if (primitiveType == "uint16")  { ((ArraySchema<uint16_t> *)value)->setAt(newIndex, decodeUint16(bytes, it)); }
                        else if (primitiveType == "int32")   { ((ArraySchema<int32_t> *)value)->setAt(newIndex, decodeInt32(bytes, it)); }
                        else if (primitiveType == "uint32")  { ((ArraySchema<uint32_t> *)value)->setAt(newIndex, decodeUint32(bytes, it)); }
                        else if (primitiveType == "int64")   { ((ArraySchema<int64_t> *)value)->setAt(newIndex, decodeInt64(bytes, it)); }
                        else if (primitiveType == "uint64")  { ((ArraySchema<uint64_t> *)value)->setAt(newIndex, decodeUint64(bytes, it)); }
                        else if (primitiveType == "float32") { ((ArraySchema<float32_t> *)value)->setAt(newIndex, decodeFloat32(bytes, it)); }
                        else if (primitiveType == "float64") { ((ArraySchema<float64_t> *)value)->setAt(newIndex, decodeFloat64(bytes, it)); }
                        else { throw std::invalid_argument("cannot decode invalid type: " + primitiveType); }
                    }

                    if (isNew)
                    {
                        if (valueRef->onAdd)
                        {
                            valueRef->onAdd(value->items.at(newIndex), newIndex);
                        }
                    }
                    else if (valueRef->onChange)
                    {
                        valueRef->onChange(value->items.at(newIndex), newIndex);
                    }

                }

                this->setArray(field, value);
#ifdef COLYSEUS_DEBUG
                std::cout << "array set successfully! size => " << value->size() << std::endl;
#endif
            }
            else if (type == "map")
            {
#ifdef COLYSEUS_DEBUG
                std::cout << "Let's call getMap for " << field << std::endl;
#endif

                MapSchema<char *>* valueRef = this->getMap(field);
                MapSchema<char *>* value = valueRef; //valueRef.clone();

                int length = (int) decodeNumber(bytes, it);
                hasChange = (length > 0);

#ifdef COLYSEUS_DEBUG
                std::cout << "MAP, LENGTH => " << length << std::endl;
#endif

                bool hasIndexChange = false;
                bool isSchemaType = this->_childSchemaTypes.find(index) != this->_childSchemaTypes.end();
#ifdef COLYSEUS_DEBUG
                std::cout << "MAP, IS SCHEMA TYPE? => " << isSchemaType << std::endl;
#endif

                // List of previous keys
                std::vector<string> previousKeys = valueRef->keys();

                for (int i = 0; i < length; i++)
                {
                    if (it->offset > totalBytes || bytes[it->offset] == (unsigned char)SPEC::END_OF_STRUCTURE)
                    {
#ifdef COLYSEUS_DEBUG
                        std::cout << "MAP: END OF STRUCTURE!" << std::endl;
#endif
                        break;
                    }

                    bool isNilItem = nilCheck(bytes, it);
                    if (isNilItem) { it->offset++; }

                    string previousKey = "";
                    if (indexChangeCheck(bytes, it)) {
                        it->offset++;
                        previousKey = previousKeys[decodeNumber(bytes, it)];
                        hasIndexChange = true;
                    }

                    bool hasMapIndex = numberCheck(bytes, it);
                    string newKey = (hasMapIndex)
                        ? previousKeys[decodeNumber(bytes, it)]
                        : decodeString(bytes, it);

#ifdef COLYSEUS_DEBUG
                    std::cout << "previousKey => " << previousKey << std::endl;
                    std::cout << "newKey => " << newKey << std::endl;
#endif

                    char* item = nullptr;
                    bool isNew = (!hasIndexChange && !valueRef->has(newKey)) || (hasIndexChange && previousKey == "" && hasMapIndex);

#ifdef COLYSEUS_DEBUG
                    std::cout << "isNew => " << isNew << std::endl;
#endif

                    if (isNew && isSchemaType)
                    {
                        item = (char*) this->createInstance(this->_childSchemaTypes.at(index));

                    } else if (previousKey != "")
                    {
                        item = valueRef->at(previousKey);

                    } else
                    {
                        if (valueRef->has(newKey)) {
                            item = valueRef->at(newKey);
                        }
                    }

                    if (isNilItem)
                    {
                        if (isSchemaType && item != nullptr && ((Schema*)item)->onRemove)
                        {
                            ((Schema *)item)->onRemove();
                        }

                        if (valueRef->onRemove) {
                            valueRef->onRemove(item, newKey);
                        }

                        value->items.erase(newKey);
                        continue;

                    } else if (!isSchemaType)
                    {
                        string primitiveType = this->_childPrimitiveTypes.at(index);

                        // FIXME: this is ugly and repetitive
                        if (primitiveType == "string")       {((MapSchema<string> *)value)->items[newKey] = decodeString(bytes, it); }
                        else if (primitiveType == "number")  {((MapSchema<varint_t> *)value)->items[newKey] = decodeNumber(bytes, it); }
                        else if (primitiveType == "boolean") { ((MapSchema<bool> *)value)->items[newKey] = decodeBoolean(bytes, it) ; }
                        else if (primitiveType == "int8")    { ((MapSchema<int8_t> *)value)->items[newKey] = decodeInt8(bytes, it) ; }
                        else if (primitiveType == "uint8")   { ((MapSchema<uint8_t> *)value)->items[newKey] = decodeUint8(bytes, it) ; }
                        else if (primitiveType == "int16")   { ((MapSchema<int16_t> *)value)->items[newKey] = decodeInt16(bytes, it) ; }
                        else if (primitiveType == "uint16")  { ((MapSchema<uint16_t> *)value)->items[newKey] = decodeUint16(bytes, it) ; }
                        else if (primitiveType == "int32")   { ((MapSchema<int32_t> *)value)->items[newKey] = decodeInt32(bytes, it) ; }
                        else if (primitiveType == "uint32")  { ((MapSchema<uint32_t> *)value)->items[newKey] = decodeUint32(bytes, it) ; }
                        else if (primitiveType == "int64")   { ((MapSchema<int64_t> *)value)->items[newKey] = decodeInt64(bytes, it) ; }
                        else if (primitiveType == "uint64")  { ((MapSchema<uint64_t> *)value)->items[newKey] = decodeUint64(bytes, it) ; }
                        else if (primitiveType == "float32") { ((MapSchema<float32_t> *)value)->items[newKey] = decodeFloat32(bytes, it) ; }
                        else if (primitiveType == "float64") { ((MapSchema<float64_t> *)value)->items[newKey] = decodeFloat64(bytes, it) ; }
                        else { throw std::invalid_argument("cannot decode invalid type: " + primitiveType); }


                    }
                    else
                    {
                        ((Schema*) item)->decode(bytes, totalBytes, it);
                        value->insert(newKey, item);
                    }

                    if (isNew)
                    {
                        if (valueRef->onAdd)
                        {
                            valueRef->onAdd(item, newKey);
                        }
                    }
                    else if (valueRef->onChange)
                    {
                        valueRef->onChange(item, newKey);
                    }
                }

                this->setMap(field, value);

            }
            else
            {
                this->decodePrimitiveType(field, type, bytes, it);
                hasChange = true;
            }
#ifdef COLYSEUS_DEBUG
            std::cout << "stepped out (child type decoding)" << std::endl;
#endif

            if (hasChange && this->onChange)
            {
                DataChange dataChange = DataChange();
                dataChange.field = field;
                // dataChange.value = value;

                changes.push_back(dataChange);
            }
        }
#ifdef COLYSEUS_DEBUG
        std::cout << "stepped out (structure)." << std::endl;
#endif

        // trigger onChange callback.
        if (this->onChange)
        {
#ifdef COLYSEUS_DEBUG
            std::cout << "let's trigger changes!" << std::endl;
#endif
            this->onChange(changes);
        }

        if (doesOwnIterator) {
#ifdef COLYSEUS_DEBUG
            std::cout << "let's delete iterator..." << std::endl;
#endif
            delete it;
        }
#ifdef COLYSEUS_DEBUG
        std::cout << "end of decode()" << std::endl;
#endif
    }

  protected:
    std::map<unsigned char, string> _indexes;
    std::map<unsigned char, string> _types;
    std::map<unsigned char, string> _childPrimitiveTypes;
    std::map<unsigned char, std::type_index> _childSchemaTypes;

    // typed virtual getters by field
    virtual string getString(const string &field) { return ""; }
    virtual varint_t getNumber(const string &field) { return 0; }
    virtual bool getBoolean(const string &field) { return 0; }
    virtual int8_t getInt8(const string &field) { return 0; }
    virtual uint8_t getUint8(const string &field) { return 0; }
    virtual int16_t getInt16(const string &field) { return 0; }
    virtual uint16_t getUint16(const string &field) { return 0; }
    virtual int32_t getInt32(const string &field) { return 0; }
    virtual uint32_t getUint32(const string &field) { return 0; }
    virtual int64_t getInt64(const string &field) { return 0; }
    virtual uint64_t getUint64(const string &field) { return 0; }
    virtual float32_t getFloat32(const string &field) { return 0; }
    virtual float64_t getFloat64(const string &field) { return 0; }
    virtual Schema* getRef(const string &field) { return nullptr; }
    virtual ArraySchema<char *> *getArray(const string &field) { return new ArraySchema<char *>(); }
    virtual MapSchema<char *> *getMap(const string &field) { return new MapSchema<char *>(); }

    // typed virtual setters by field
    virtual void setString(const string &field, string value) {}
    virtual void setNumber(const string &field, varint_t value) {}
    virtual void setBoolean(const string &field, bool value) {}
    virtual void setInt8(const string &field, int8_t value) {}
    virtual void setUint8(const string &field, uint8_t value) {}
    virtual void setInt16(const string &field, int16_t value) {}
    virtual void setUint16(const string &field, uint16_t value) {}
    virtual void setInt32(const string &field, int32_t value) {}
    virtual void setUint32(const string &field, uint32_t value) {}
    virtual void setInt64(const string &field, int64_t value) {}
    virtual void setUint64(const string &field, uint64_t value) {}
    virtual void setFloat32(const string &field, float32_t value) {}
    virtual void setFloat64(const string &field, float64_t value) {}
    virtual void setRef(const string &field, Schema* value) {}
    virtual void setArray(const string &field, ArraySchema<char*>*) {}
    virtual void setMap(const string &field, MapSchema<char*>*) {}

    virtual Schema* createInstance(std::type_index type) { return nullptr; }

  private:
    inline void decodePrimitiveType(const string &field, string type, unsigned const char bytes[], Iterator *it)
    {
        if (type == "string")       { this->setString(field, decodeString(bytes, it)); }
        else if (type == "number")  { this->setNumber(field, decodeNumber(bytes, it)); }
        else if (type == "boolean") { this->setBoolean(field, decodeBoolean(bytes, it)); }
        else if (type == "int8")    { this->setInt8(field, decodeInt8(bytes, it)); }
        else if (type == "uint8")   { this->setUint8(field, decodeUint8(bytes, it)); }
        else if (type == "int16")   { this->setInt16(field, decodeInt16(bytes, it)); }
        else if (type == "uint16")  { this->setUint16(field, decodeUint16(bytes, it)); }
        else if (type == "int32")   { this->setInt32(field, decodeInt32(bytes, it)); }
        else if (type == "uint32")  { this->setUint32(field, decodeUint32(bytes, it)); }
        else if (type == "int64")   { this->setInt64(field, decodeInt64(bytes, it)); }
        else if (type == "uint64")  { this->setUint64(field, decodeUint64(bytes, it)); }
        else if (type == "float32") { this->setFloat32(field, decodeFloat32(bytes, it)); }
        else if (type == "float64") { this->setFloat64(field, decodeFloat64(bytes, it)); }
        else { throw std::invalid_argument("cannot decode invalid type: " + type); }
    }
};

} // namespace schema
} // namespace colyseus

