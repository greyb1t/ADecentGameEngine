#pragma once

#include <array>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <string>

namespace Common
{
	extern const std::array<int, 10000> globalPrimeNumbers;

	template <class Key, class Value>
	class HashMap
	{
	public:
		//Skapar en HashMap med plats för aCapacity element. Den behöver inte
		//kunna växa. Blir den full så blir den full.
		HashMap(int aCapacity);

		//Stoppar in aValue under nyckeln aKey. Om det fanns ett värde innan med
		//aKey som nyckel så skrivs det värdet över. Om det lyckas returneras
		//true. Om map:en är full så görs ingenting, och false returneras.
		bool Insert(const Key& aKey, const Value& aValue);

		//Plockar bort elementet med nyckeln aKey, om det finns. Om det finns
		//returneras true. Om det inte finns görs ingenting, och false
		//returneras.
		bool Remove(const Key& aKey);

		//Ger en pekare till värdet med nyckeln aKey, om det finns. Om det inte
		//finns returneras nullptr.
		const Value* Get(const Key& aKey) const;

		//Som ovan, men returnerar en icke-const-pekare.
		Value* Get(const Key& aKey);

	private:
		enum class State : uint8_t
		{
			Empty = 0,
			InUse,
			Removed
		};

		struct Entry
		{
			State myState;
			Key myKey;
			Value myValue;
		};

		int FindPos(const Key& aKey);

		bool Equals(const Key& aKey1, const Key& aKey2) const;

		int NextPrime(const int aValue);

		uint32_t MyHash(const Key& aKey) const;

		// Returns -1 if not found
		int GetIndex(const Key& aKey) const;

		uint32_t StdHash(const uint8_t* aBuffer, const int aCount) const;

		template <typename T>
		uint32_t Hash(const T& aValue) const;

		template <>
		uint32_t Hash<std::string>(const std::string& aValue) const
		{
			return StdHash(reinterpret_cast<const uint8_t*>(aValue.data()), aValue.size());
		}

		std::vector<Entry> myEntries;
		int myFakeSize = 0;
		int myElementCount = 0;
	};

	template<class Key, class Value>
	inline HashMap<Key, Value>::HashMap(int aCapacity)
	{
		myFakeSize = aCapacity * 2;

		myEntries.resize(NextPrime(aCapacity * 2));

		myElementCount = 0;

		for (auto& entry : myEntries)
		{
			entry.myState = State::Empty;
		}
	}

	template <class Key, class Value>
	bool Common::HashMap<Key, Value>::Insert(const Key& aKey, const Value& aValue)
	{
		if (myFakeSize <= 0)
		{
			return false;
		}

		const int index = FindPos(aKey);

		const bool insertingNew =
			myEntries[index].myState == State::Empty ||
			myEntries[index].myState == State::Removed;

		if (insertingNew)
		{
			const bool isFull = myElementCount >= (myFakeSize / 2);

			if (isFull)
			{
				return false;
			}

			++myElementCount;

			myEntries[index].myKey = std::move(aKey);
			myEntries[index].myState = State::InUse;
		}

		myEntries[index].myValue = std::move(aValue);

		return true;
	}

	template <class Key, class Value>
	bool Common::HashMap<Key, Value>::Remove(const Key& aKey)
	{
		if (myFakeSize <= 0)
		{
			return false;
		}

		const int index = GetIndex(aKey);

		if (index != -1)
		{
			myEntries[index].myState = State::Removed;

			return true;
		}

		return false;
	}

	template <class Key, class Value>
	const Value* Common::HashMap<Key, Value>::Get(const Key& aKey) const
	{
		return Get(aKey);
	}

	template <class Key, class Value>
	Value* Common::HashMap<Key, Value>::Get(const Key& aKey)
	{
		if (myFakeSize <= 0)
		{
			return nullptr;
		}

		const int index = GetIndex(aKey);

		if (index != -1)
		{
			return &myEntries[index].myValue;
		}

		return nullptr;
	}

	template <class Key, class Value>
	int Common::HashMap<Key, Value>::FindPos(const Key& aKey)
	{
		int offset = 1;

		int index = MyHash(aKey);

		while (true)
		{
			if (myEntries[index].myState == State::Empty)
			{
				return index;
			}
			else if (myEntries[index].myState == State::InUse &&
				Equals(myEntries[index].myKey, aKey))
			{
				return index;
			}
			else if (myEntries[index].myState == State::Removed &&
				Equals(myEntries[index].myKey, aKey))
			{
				return index;
			}

			index += offset;
			offset += 2;

			const int size = static_cast<int>(myEntries.size());

			if (index >= size)
			{
				index -= size;
			}
		}

		return index;
	}

	template <class Key, class Value>
	bool Common::HashMap<Key, Value>::Equals(const Key& aKey1, const Key& aKey2) const
	{
		return !(aKey1 < aKey2 || aKey2 < aKey1);
	}

	template<class Key, class Value>
	inline int HashMap<Key, Value>::NextPrime(const int aValue)
	{
		// Edge case because if aValue is 0, next prime becomes 2, which is incorrect
		if (aValue == 0)
		{
			return 0;
		}

		const auto primeIt = std::lower_bound(globalPrimeNumbers.begin(), globalPrimeNumbers.end(), aValue);

		assert(primeIt != globalPrimeNumbers.end() && "The prime was not in the database");

		return *primeIt;
	}

	template <class Key, class Value>
	uint32_t Common::HashMap<Key, Value>::MyHash(const Key& aKey) const
	{
		return Hash(aKey) % myEntries.size();
	}

	template <class Key, class Value>
	int Common::HashMap<Key, Value>::GetIndex(const Key& aKey) const
	{
		int offset = 1;

		int index = MyHash(aKey);

		while (true)
		{
			if (myEntries[index].myState == State::Empty)
			{
				return -1;
			}
			else if (myEntries[index].myState == State::InUse &&
				Equals(myEntries[index].myKey, aKey))
			{
				return index;
			}
			else if (myEntries[index].myState == State::Removed &&
				Equals(myEntries[index].myKey, aKey))
			{
				return -1;
			}

			index += offset;
			offset += 2;

			const int size = static_cast<int>(myEntries.size());

			if (index >= size)
			{
				index -= size;
			}
		}

		return index;
	}

	template <class Key, class Value>
	uint32_t Common::HashMap<Key, Value>::StdHash(const uint8_t* aBuffer, const int aCount) const
	{
		const uint32_t FNVOffsetBasis = 2166136261U;
		const uint32_t FNVPrime = 16777619U;
		uint32_t val = FNVOffsetBasis;

		for (int i = 0; i < aCount; ++i)
		{
			val ^= aBuffer[i];
			val *= FNVPrime;
		}

		return val;
	}

	template <class Key, class Value>
	template <typename T>
	uint32_t Common::HashMap<Key, Value>::Hash(const T& aValue) const
	{
		return StdHash(reinterpret_cast<const uint8_t*>(&aValue), sizeof(T));
	}
}

namespace CU = Common;