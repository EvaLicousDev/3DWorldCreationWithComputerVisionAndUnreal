#pragma once

//Function template to allow creation of array of adresses for elements of same class
template <typename ...ElementType>
constexpr std::array<ElementType*, sizeof...(ElementType)> getAdressArray(ElementType&&... elements)
{
    return { &elements... };
}