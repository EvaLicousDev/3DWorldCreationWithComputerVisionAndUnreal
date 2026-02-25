// #pragma once
// #include <array>
//
// //Function template to allow creation of array of adresses for elements of same class
//  // template <typename First, typename ...ElementType>
//  // constexpr void getAdressArray(size_t index, First* outArray[], First first, ElementType&... elements)
//  // {
//  //     First* ptrFirst = &first; 
//  //     outArray[index] = ptrFirst; 
//  //     ++index;
//  //     if (sizeof...(elements) > 0 && index < sizeof(outArray))
//  //     {
//  //         getAdressArray(index, outArray, elements...);
//  //     }
//  // }
//
// template <typename First, typename ARRAY, typename ...ElementType>
// constexpr std::array<std::unique_ptr<First>, sizeof...(ElementType)> 
// addIntoUniqueArray(size_t arraySize, ARRAY& array, First&& first, size_t index, ElementType&&... elements)
// {
//     array[index] = std::make_unique<First>(first);
//     index++; 
//     if (index < sizeof(array - 1))
//     {
//         return addIntoUniqueArray(arraySize, std::move<ARRAY>(array), ++index, std::forward<ElementType>(elements)...); 
//     }
//     else
//     {
//         return std::move(array); 
//     }
// }
//
// //Function template to allow creation of array of smart pointers
// template <typename First, typename ...ElementType>
// constexpr std::array<std::unique_ptr<First>, (sizeof...(ElementType)+1)>
// createUniquePtrArray(First&& first, ElementType&&... elements)
// {
//     auto arraySize = (sizeof...(ElementType) + 1);
//     std::array<std::unique_ptr<First>, arraySize> outputArray;
//     auto index = 0;
//     outputArray[index] = std::make_unique<First>(first);
//     return addIntoUniqueArray(arraySize, std::forward<std::array<std::unique_ptr<First>, arraySize>>(outputArray),++index, std::forward<ElementType>(elements)...);
// }