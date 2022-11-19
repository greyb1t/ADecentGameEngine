#pragma once

#include <memory>

/*
	Example usage:

	Owned<int> myInt = MakeOwned<int>(0);

	Owned<Tga2D::CSprite> sprite = MakeOwned<Tga2D::CSprite>("path");
*/

template <typename T>
using Owned = std::unique_ptr<T>;

// Only accepts rvalues
template <typename T, typename... Args>
inline Owned<T> MakeOwned(Args&&... aArgs)
{
	return std::make_unique<T>(std::forward<Args>(aArgs)...);
}

template <typename T>
using Shared = std::shared_ptr<T>;

// Only accepts rvalues
template <typename T, typename... Args>
inline Shared<T> MakeShared(Args&&... aArgs)
{
	return std::make_shared<T>(std::forward<Args>(aArgs)...);
}

template <typename T>
using Weak = std::weak_ptr<T>;