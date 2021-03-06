#ifndef XECS_ARCHETYPE_HPP
#define XECS_ARCHETYPE_HPP

#include <cstdlib>
#include <limits>
#include <tuple>
#include <type_traits>

namespace xecs
{
/**
 * @brief List of types.
 * 
 * Similar to a tuple, but cannot contain any data. 
 * This is used heavily for our compile-time meta programming.
 * 
 * @tparam Types All the types to be contained by the list
 */
template<typename... Types>
struct list
{};

/**
 * @brief Finds size of a list (amount of types).
 * 
 * @tparam List List of types
 */
template<typename List>
struct size;

template<typename... Types, template<typename...> class List>
struct size<List<Types...>> : std::integral_constant<size_t, sizeof...(Types)>
{};

template<typename List>
constexpr auto size_v = size<List>::value;

/**
 * @brief Checks if a list is empty.
 * 
 * @tparam List List of types
 */
template<typename List>
struct empty;

template<typename... Types, template<typename...> class List>
struct empty<List<Types...>> : std::bool_constant<sizeof...(Types) == 0>
{};

template<typename List>
constexpr auto empty_v = empty<List>::value;

/**
 * @brief Finds the index of the specified type in the list.
 * 
 * Returns size of list if nothing was found.
 * 
 * @tparam Type the type to find
 * @tparam List The list to operate on
 * @tparam I Current search index (default start at 0)
 */
template<typename Type, typename List, size_t I = 0>
struct find;

template<typename Type, typename... Types, template<typename...> class List, size_t I>
struct find<Type, List<Types...>, I> : std::integral_constant<size_t, I>
{};

template<typename Type, typename Head, typename... Types, template<typename...> class List, size_t I>
struct find<Type, List<Head, Types...>, I>
  : std::conditional_t<std::is_same_v<Type, Head>, std::integral_constant<size_t, I>, find<Type, List<Types...>, I + 1>>
{};

template<typename Type, typename List>
constexpr auto find_v = find<Type, List, 0>::value;

/**
 * @brief Finds the type at the specified index in the list.
 * 
 * @tparam I Index in the list
 * @tparam List The list to operate on
 */
template<size_t I, typename List>
struct at;

template<size_t I, typename... Types, template<typename...> class List>
struct at<I, List<Types...>>
{
  using type = List<>;
};

template<size_t I, typename Type, typename... Types, template<typename...> class List>
struct at<I, List<Type, Types...>>
{
private:
  using next = typename at<I - 1, List<Types...>>::type;

public:
  using type = typename std::conditional_t<I == 0, Type, next>;
};

template<size_t I, typename List>
using at_t = typename at<I, List>::type;

/**
 * @brief Checks if a list contains a type.
 * 
 * @tparam Type Type to check for
 * @tparam List List to check
 */
template<typename Type, typename List>
struct contains;

template<typename Type, typename... Types, template<typename...> class List>
struct contains<Type, List<Types...>> : std::disjunction<std::is_same<Type, Types>...>
{};

/**
 * @brief Utility for contains value.
 * 
 * @tparam Type Type to check for
 * @tparam List List to check
 */
template<typename Type, typename List>
constexpr auto contains_v = contains<Type, List>::value;

/**
 * @brief Checks if list contains atleast all required types.
 * 
 * @tparam List List to check
 * @tparam Types Required types
 */
template<typename List, typename... Types>
struct contains_all;

template<typename List, typename... Types>
struct contains_all : std::conjunction<contains<Types, List>...>
{};

template<typename List, typename... Types>
constexpr auto contains_all_v = contains_all<List, Types...>::value;

/**
 * @brief Checks if all types are unique.
 * 
 * Types are determined not to be the same by std::is_same<T1, T2>.
 * 
 * @tparam Types Types to check
 */
template<typename... Types>
struct unique_types : std::true_type
{};

template<typename Type, typename... Types>
struct unique_types<Type, Types...>
  : std::conjunction<std::negation<std::disjunction<std::is_same<Type, Types>...>>, unique_types<Types...>>
{};

template<typename... Types>
constexpr auto unique_types_v = unique_types<Types...>::value;

/**
 * @brief Checks if two lists contain the same types.
 * 
 * Compares the size of both lists and checks if Blist contains all the types of AList.
 * 
 * @tparam AList First list of types
 * @tparam BList Second list of types
 */
template<typename AList, typename BList>
struct is_same_types;

template<typename... ATypes, template<typename...> class AList, typename... BTypes, template<typename...> class BList>
struct is_same_types<AList<ATypes...>, BList<BTypes...>>
  : std::conjunction<std::bool_constant<sizeof...(ATypes) == sizeof...(BTypes)>, contains_all<AList<ATypes...>, BTypes...>>
{};

template<typename AList, typename BList>
constexpr auto is_same_types_v = is_same_types<AList, BList>::value;

/**
 * @brief Checks if all lists are unique in terms of types.
 * 
 * This is different than unique_types because it ignores the order of
 * types contained by a list. unique_types would evaluate a list with the same types but in a
 * different order as a different list.
 * 
 * @tparam Lists A list of lists
 */
template<typename... Lists>
struct unique_lists : std::true_type
{};

template<typename List, typename... Lists>
struct unique_lists<List, Lists...>
  : std::conjunction<std::negation<std::disjunction<is_same_types<List, Lists>...>>, unique_lists<Lists...>>
{};

template<typename... Lists>
constexpr auto unique_lists_v = unique_lists<Lists...>::value;

/**
 * @brief Adds a type to the front of a list.
 * 
 * @tparam Type Type to add
 * @tparam List List to add type to
 */
template<typename Type, typename List>
struct push_front;

template<typename Type, typename... Types, template<typename...> class List>
struct push_front<Type, List<Types...>>
{
  using type = List<Type, Types...>;
};

template<typename Type, typename List>
using push_front_t = typename push_front<Type, List>::type;

/**
 * @brief Adds a type to the back of a list.
 * 
 * @tparam Type Type to add
 * @tparam List List to add type to
 */
template<typename Type, typename List>
struct push_back;

template<typename Type, typename... Types, template<typename...> class List>
struct push_back<Type, List<Types...>>
{
  using type = List<Types..., Type>;
};

template<typename Type, typename List>
using push_back_t = typename push_back<Type, List>::type;

/**
 * @brief Finds the first occurence of a list that.
 * 
 * This uses is_same_types, so the list to be found must contain all the specified types,
 * and only those types, nothing more, nothing less.
 * 
 * This is used to find the correct type of list with the specified components for any given order.
 * 
 * @tparam ListOfLists A List of lists to search
 * @tparam Types Exact contained types of the list to find.
 */
template<typename ListOfLists, typename... Types>
struct find_for;

template<typename... Lists, template<typename...> class ListOfLists, typename... Types>
struct find_for<ListOfLists<Lists...>, Types...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> class ListOfLists, typename... Types>
struct find_for<ListOfLists<HeadList, Lists...>, Types...>
{
private:
  using next = typename find_for<ListOfLists<Lists...>, Types...>::type;

public:
  using type = typename std::conditional_t<is_same_types_v<HeadList, list<Types...>>, HeadList, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using find_for_t = typename find_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief Removes all lists that do not contains the required types.
 * 
 * This is used to create views at compile time.
 * 
 * @tparam ListOfLists A List of lists to prune
 * @tparam RequiredTypes types that must be present in the list
 */
template<typename ListOfLists, typename... RequiredTypes>
struct prune_for;

template<typename... Lists, template<typename...> class ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<Lists...>, RequiredTypes...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> class ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<HeadList, Lists...>, RequiredTypes...>
{
private:
  using next = typename prune_for<ListOfLists<Lists...>, RequiredTypes...>::type;
  using to_front = typename push_front<HeadList, next>::type; // Can make certain operations O(1)
  using to_back = typename push_back<HeadList, next>::type;
  using accept = typename std::conditional_t<size_v<HeadList> == sizeof...(RequiredTypes), to_front, to_back>;

public:
  using type = typename std::conditional_t<contains_all<HeadList, RequiredTypes...>::value, accept, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using prune_for_t = typename prune_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief Assert's a component to verify that is is valid.
 * 
 * A valid component cannot be cv-qualified, that means it cant be const or volatile. Also,
 * in order to use the component in our containers, we need them to be default constructible,
 * copy assignable and copy constructable.
 * 
 * We allow components that are non-trival, however you should implement move schematics
 * (move assignable & move constructable) for efficient use if your using pointers to heap data.
 * 
 * @note For the best performance, components should be a POD type (Plain old data).
 * 
 * @tparam Component The component to verify
 */
template<typename Component>
struct verify_component
{
  static_assert(std::is_same_v<Component, std::remove_cv_t<Component>>, "Component cannot be cv-qualified (const or volatile)");
  static_assert(std::is_default_constructible_v<Component>, "Component must be default constructible");
  static_assert(std::is_copy_assignable_v<Component>, "Component must be copy assignable");
  static_assert(std::is_copy_constructible_v<Component>, "Component must be copy constructable");
};

template<typename... Components>
using archetype = list<Components...>;

/**
 * @brief Assert's a archetype to verify that it is valid.
 * 
 * A valid archetype must contain only valid components determined by verify_component. Additionnaly,
 * all components in an archetype must be unique.
 * 
 * Archetypes are essentially sets of components.
 * 
 * @tparam Archetype The archetype to verify
 */
template<typename Archetype>
struct verify_archetype;

template<typename... Components>
struct verify_archetype<archetype<Components...>> : verify_component<Components>...
{
  static_assert(unique_types_v<Components...>, "Every component must be unique (archetype is a SET of components)");
};

/**
 * @brief Assert's a list of archetypes.
 * 
 * A valid list of archetypes must contain only valid archetypes determined by verify_archetype. Additionnaly,
 * all archetypes in the list must be unique.
 * 
 * For two archetypes to be different, they must be of different sizes or contains different components.
 * Archetypes with the same components but in different orders do not count as unique.
 * 
 * @tparam ArchetypeList The archetype list to verify
 */
template<typename ArchetypeList>
struct verify_archetype_list;

template<typename... Archetypes>
struct verify_archetype_list<list<Archetypes...>> : verify_archetype<Archetypes>...
{
  static_assert(unique_lists_v<Archetypes...>, "Every archetype must be unique (regardless of component order)");
};

namespace internal
{
  template<typename... Archetypes>
  struct archetype_list_builder
  {
    template<typename Archetype>
    using add = archetype_list_builder<Archetype, Archetypes...>;
    using build = list<Archetypes...>;
  };
} // namespace internal

/**
 * @brief Compile-time builder pattern for creating lists of archetypes.
 * 
 * Alternative to building archetype lists manually. This may provide better readability.
 */
struct archetype_list_builder : internal::archetype_list_builder<>
{};
} // namespace xecs

#endif