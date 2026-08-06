#pragma once


#include "BasicMath.hpp"


namespace dg
{
    using namespace Diligent;

    template<typename T>
    using Ref = RefCntAutoPtr<T>;

    template<typename T>
    using WeakRef = RefCntWeakPtr<T>;
}