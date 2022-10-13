//
// Created by Andrey Aralov on 10/13/22.
//

#include <forward_list>
#include <list>
#include <vector>
#include <gtest/gtest.h>
#include "cycled_view.hpp"


TEST( CycledViewTest, ConstAccess )
{
    const std::forward_list<int> list{ 1, 2, 3 };
    CycledView cycledView( list );

    auto listIt = list.begin();
    for ( auto it = cycledView.begin(); listIt != list.end(); ++it, ++listIt )
        ASSERT_EQ( *it, *listIt );
}

TEST( CycledViewTest, NonConstAccess )
{
    std::forward_list<int> list{ 1, 2, 3 };
    std::forward_list<int> res{ 2, 3, 4 };
    CycledView cycledView( list );

    static_assert( std::same_as<decltype( cycledView ), CycledView<std::forward_list<int>&>> );
    auto listIt = list.begin();
    for ( auto it = cycledView.begin(); listIt != list.end(); ++it, ++listIt )
    {
        *it += 1;
    }

    ASSERT_EQ( list, res );
}

TEST( CycledViewTest, ForwardIterator )
{
    const std::forward_list<int> list{ 1, 2, 3 };
    CycledView cycledView( list );

    static_assert( std::same_as<decltype( cycledView ), CycledView<const std::forward_list<int>&>> );
    static_assert( std::ranges::forward_range<decltype( cycledView )> );
    static_assert( !std::ranges::bidirectional_range<decltype( cycledView )> );
    static_assert( !std::ranges::random_access_range<decltype( cycledView )> );
    static_assert( !std::ranges::contiguous_range<decltype( cycledView )> );
}

TEST( CycledViewTest, BidirectionalIterator )
{
    const std::list<int> list{ 1, 2, 3 };
    CycledView cycledView( list );

    static_assert( std::same_as<decltype( cycledView ), CycledView<const std::list<int>&>> );
    static_assert( std::ranges::forward_range<decltype( cycledView )> );
    static_assert( std::ranges::bidirectional_range<decltype( cycledView )> );
    static_assert( !std::ranges::random_access_range<decltype( cycledView )> );
    static_assert( !std::ranges::contiguous_range<decltype( cycledView )> );
}

TEST( CycledViewTest, RandomAccessIterator )
{
    const std::vector<int> vec{ 1, 2, 3 };
    CycledView cycledView( vec );

    static_assert( std::same_as<decltype( cycledView ), CycledView<const std::vector<int>&>> );
    static_assert( std::ranges::forward_range<decltype( cycledView )> );
    static_assert( std::ranges::bidirectional_range<decltype( cycledView )> );
    static_assert( std::ranges::random_access_range<decltype( cycledView )> );
    static_assert( !std::ranges::contiguous_range<decltype( cycledView )> );
}

TEST( CycledViewTest, PerfectForwarding )
{
    CycledView cycledView( std::vector{ 1, 2, 3 } );

    static_assert( std::same_as<decltype( cycledView ), CycledView<std::vector<int>>> );
    ASSERT_EQ( cycledView[0], 1 );
    ASSERT_EQ( cycledView[1], 2 );
    ASSERT_EQ( cycledView[2], 3 );
    ASSERT_EQ( cycledView[3], 1 );
    ASSERT_EQ( cycledView[4], 2 );
    ASSERT_EQ( cycledView[5], 3 );
    ASSERT_EQ( cycledView[6], 1 );
}

TEST( CycledViewTest, CyclingLogic )
{
    const std::vector<int> vec{ 1, 2, 3 };

    {
        const std::list<int> res{ 1, 2, 3, 1, 2, 3, 1, 2 };
        auto rng = vec | cycle | std::views::take( 8 );
        std::list<int> resCycled;
        std::ranges::copy( rng, std::back_inserter( resCycled ) );
        ASSERT_EQ( res, resCycled );
    }

    {
        auto rng = vec | cycle;
        ASSERT_EQ( rng[0], 1 );
        ASSERT_EQ( rng[1], 2 );
        ASSERT_EQ( rng[2], 3 );
        ASSERT_EQ( rng[3], 1 );
        ASSERT_EQ( rng[4], 2 );
        ASSERT_EQ( rng[5], 3 );
        ASSERT_EQ( rng[6], 1 );
    }
}