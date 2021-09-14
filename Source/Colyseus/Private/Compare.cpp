#include "Compare.h"

msgpack::object_handle* Compare::emptyState = nullptr;

void msgdebug(msgpack::object obj)
{
	std::cout << obj << std::endl;
}

PatchObject::PatchObject(std::vector<std::string> path, std::string op, msgpack::object value)
{
	this->path = path;
	this->op = op;
	this->value = value;
	//    this->previousValue = previousValue;
}

bool Compare::containsKey(msgpack::object_map map, msgpack::object_kv key)
{
#ifdef COLYSEUS_DEBUG
	std::cout << "======================setState=======================" << std::endl;
	std::cout << map.ptr << std::endl;
	std::cout << key.key << std::endl;
	std::cout << key.val << std::endl;
	std::cout << "========================================================" << std::endl;
#endif

	for (uint32_t i = 0; i < map.size; i++)
	{
		if (map.ptr[i].key.type == msgpack::type::STR)
		{
			std::string key1;
			map.ptr[i].key.convert(key1);

			if (key.key.type == msgpack::type::STR)
			{
				std::string key2;
				key.key.convert(key2);
				if (key1 == key2)
				{
					return true;
				}
			}
		}
	}
	return false;
}

std::vector<PatchObject> Compare::getPatchList(const msgpack::object tree1, const msgpack::object tree2)
{
	std::vector<PatchObject> patches;
	std::vector<std::string> path;

	generate(tree1, tree2, &patches, path);
	return patches;
}

// Dirty check if obj is different from mirror, generate patches and update mirror
void Compare::generate(const msgpack::object mirrorPacked, const msgpack::object objPacked, std::vector<PatchObject>* patches,
	std::vector<std::string> path)
{
#ifdef COLYSEUS_DEBUG
	std::cout << "----------------------- Compare::generate ------------------------" << std::endl;
	std::cout << "OLD: " << mirrorPacked << std::endl;
	std::cout << "NEW: " << objPacked << std::endl;
	std::cout << "------------------------------------------------------------------" << std::endl;
#endif

	msgpack::object_map mirror = mirrorPacked.via.map;
	msgpack::object_map obj = objPacked.via.map;

	auto newKeys = obj.ptr;
	auto oldKeys = mirror.ptr;
	bool deleted = false;
	for (int i = mirror.size - 1; i >= 0; i--)
	{
		msgpack::object_kv kv = oldKeys[i];

		if (containsKey(obj, kv) && containsKey(mirror, kv) && objPacked.type != msgpack::type::ARRAY)
		{
			auto oldVal = mirror.ptr[i].val;
			auto newVal = obj.ptr[i].val;

			if (oldVal.type == msgpack::type::MAP && newVal.type == msgpack::type::MAP)
			{
				std::vector<std::string> deeperPath(path);
				std::string aa;
				kv.key.convert(aa);
				deeperPath.push_back(aa);
				generate(oldVal, newVal, patches, deeperPath);
			}
			else
			{
				if (oldVal != newVal)
				{
					std::vector<std::string> replacePath(path);
					std::string aa;
					kv.key.convert(aa);
					replacePath.push_back(aa);
					patches->push_back(PatchObject(replacePath, "replace", newVal));
				}
			}
		}

		else
		{
			std::vector<std::string> removePath(path);
			std::string aa;
			kv.key.convert(aa);
			removePath.push_back(aa);
			msgpack::object val;
			patches->push_back(PatchObject(removePath, "remove", val));
			deleted = true;	   // property has been deleted
		}
	}

	if (!deleted && obj.size == mirror.size)
	{
		return;
	}

	for (int i = obj.size - 1; i >= 0; i--)
	{
		msgpack::object_kv kv = newKeys[i];
		if (!containsKey(mirror, kv) && containsKey(obj, kv))
		{
			std::vector<std::string> addPath(path);
			std::string aa;
			kv.key.convert(aa);
			addPath.push_back(aa);

			auto newVal = obj.ptr[i].val;

			// compare deeper additions
			if ((newVal.type == msgpack::type::MAP || newVal.type == msgpack::type::ARRAY) && newVal.type != msgpack::type::NIL)
			{
				generate(emptyState->get(), newVal, patches, addPath);
			}

			patches->push_back(PatchObject(addPath, "add", newVal));
		}
	}
}
