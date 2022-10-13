//
// Created by Andrey Aralov on 10/12/22.
//
#pragma once

#include <iterator>
#include <ranges>


// Require at least forward range since to cycle the container
// it has to be multi-passable.
template <std::ranges::forward_range TRange>
class CycledView : public std::ranges::view_interface<CycledView<TRange>>
{
    using UnderlyingIterator = typename std::ranges::iterator_t<TRange>;
public:
    CycledView() = default;

    template <typename T>
    explicit CycledView( T&& range ):
        begin_( std::ranges::begin( range ) ),
        end_( std::ranges::end( range ) )
    {}

    auto begin()
    {
        return Iterator( this );
    }

    std::default_sentinel_t end() const
    { return {}; }

private:
    UnderlyingIterator begin_, end_;

    struct Iterator {
        using iterator_traits = std::iterator_traits<UnderlyingIterator>;
        using iterator_category = typename iterator_traits::iterator_category;
        using difference_type = typename iterator_traits::difference_type;
        using value_type = typename iterator_traits::value_type;
        using pointer = typename iterator_traits::pointer;
        using reference = typename iterator_traits::reference;

        static constexpr bool is_read_only_v = std::is_convertible<decltype( *std::declval<UnderlyingIterator>() ), value_type&>::value;


        Iterator() = default;

        Iterator( CycledView<TRange>* view ) noexcept:
            view_( view ),
            current_( view_->begin_ )
        {}

        const reference operator*() const
        {
            return *current_;
        }

        // non-const dereference is available if only
        // underlying range supports it
        reference operator*()
            requires ( !is_read_only_v )
        {
            return *current_;
        }


        // forward iterator
        bool operator==( const Iterator& other ) const
        {
            return current_ == other.current_;
        }

        bool operator==( std::default_sentinel_t sentinel ) const
        {
            return false;
        }

        Iterator& operator++()
            requires std::forward_iterator<UnderlyingIterator>
        {
            if ( ++current_ == view_->end_ )
                current_ = view_->begin_;

            return *this;
        }

        Iterator operator++( int )
            requires std::forward_iterator<UnderlyingIterator>
        {
            auto ret = *this;
            ++( *this );
            return ret;
        }
        // forward iterator


        // bidirectional iterator
        Iterator& operator--()
            requires std::bidirectional_iterator<UnderlyingIterator>
        {
            if ( current_ == view_->begin_ )
                current_ = std::prev( view_->end_ );

            return *this;
        }

        Iterator operator--( int )
            requires std::bidirectional_iterator<UnderlyingIterator>
        {
            auto ret = *this;
            --( *this );
            return ret;
        }
        // bidirectional iterator


        // random access iterator
        Iterator& operator+=( difference_type n )
            requires std::random_access_iterator<UnderlyingIterator>
        {
            if ( n < 0 )
                return *( this ) -= std::abs( n );

            const auto size = std::distance( view_->begin_, view_->end_ );
            const auto dist_to_end = std::distance( current_, view_->end_ );

            if ( n % size < dist_to_end )
                current_ += n % size;
            else
                current_ = view_->begin_ + ( n % size - dist_to_end );

            return *this;
        }

        Iterator operator+( difference_type n ) const
            requires std::random_access_iterator<UnderlyingIterator>
        {
            auto other = *this;
            other += n;
            return other;
        }

        Iterator& operator-=( difference_type n )
            requires std::random_access_iterator<UnderlyingIterator>
        {
            if ( n < 0 )
                return *( this ) += std::abs( n );

            const auto size = std::distance( view_->begin_, view_->end_ );
            const auto dist_from_begin = std::distance( view_->begin_, current_ );

            if ( n % size < dist_from_begin )
                current_ -= n % size;
            else
                current_ = view_->end_ - ( n % size - dist_from_begin );

            return *this;
        }

        Iterator operator-( difference_type n ) const
            requires std::random_access_iterator<UnderlyingIterator>
        {
            auto other = *this;
            other -= n;
            return other;
        }

        friend Iterator operator+( difference_type n, const Iterator& iter )
        {
            return iter + n;
        }

        const reference operator[]( difference_type n ) const
            requires std::random_access_iterator<UnderlyingIterator>
        {
            return *( *this + n );
        }

        reference operator[]( difference_type n )
            requires ( std::random_access_iterator<UnderlyingIterator>
                       && !is_read_only_v )
        {
            return *( *this + n );
        }

        auto operator<=>( const Iterator& other ) const
            requires std::random_access_iterator<UnderlyingIterator>
        {
            return current_ <=> other.current_;
        }

        difference_type operator-( const Iterator& other ) const
            requires std::random_access_iterator<UnderlyingIterator>
        {
            return std::min( std::ranges::distance( view_->begin_, current_ ),
                             std::ranges::distance( current_, view_->end_ ) );
        }
        // random access iterator


    private:
        CycledView<TRange>* view_;
        UnderlyingIterator current_;
    };
};


template <typename T>
CycledView( T&& range ) -> CycledView<std::remove_reference_t<T>>;

struct CycledViewFn
{
    template <std::ranges::input_range TRange>
    auto operator()( TRange&& range ) const
    {
        return CycledView( std::forward<TRange>( range ) );
    }

    template <std::ranges::input_range TRange>
    friend auto operator| ( TRange&& range, const CycledViewFn& cycledViewFn )
    {
        return CycledView( std::forward<TRange>( range ) );
    }
} cycle;