/*
 * multi_array.hpp file
 * 
 * Copyright (C) 2014 Lukas Hermanns
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef _CPPLIBEXT_MULTI_ARRAY_H_
#define _CPPLIBEXT_MULTI_ARRAY_H_


#include <cpplibext/detail/product.hpp>
#include <cpplibext/detail/select.hpp>

#include <initializer_list>
#include <limits>
#include <algorithm>
#include <array>


namespace ext
{


/**
Multi dimensional array class.
\tparam T Specifies the data type for the array elements.
\tparam Dimensions... Specifies the array dimensions. This must be at least 1 entry.
*/
template <class T, std::size_t... Dimensions> class multi_array
{
    
    public:

        typedef T                   value_type;
        typedef std::size_t         size_type;
        typedef std::ptrdiff_t      difference_type;
        typedef value_type&         reference;
        typedef const value_type&   const_reference;
        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;

    private:
        
        /*
        Helper structures with template meta programming
        to determine array size and size of slices
        */

        /* --- Array size (Dimension1 * Dimension2 * ... * DimensionN) --- */

        template <std::size_t... DimN> struct array_size
        {
            static const size_type value = detail::product<size_type, DimN...>::value;
        };

        /* --- Next array size (Dimension2 * Dimension3 * ... * DimensionN) --- */

        template <std::size_t FirstDimension, std::size_t... DimN> struct next_array_size
        {
            /* Ignore first dimension here, just use the variadic template arguments */
            static const size_type value = detail::product<size_type, DimN...>::value;
        };

        /* --- First dimension --- */

        template <std::size_t FirstDimension, std::size_t... DimN> struct first_dimension
        {
            /* Ignore variadic template arguments here, just use the first dimension */
            static const size_type value = FirstDimension;
        };

        /* --- next_dimension --- */

        // Declaration for GCC and clang
        template <std::size_t... DimN> struct next_dimension_secondary;

        template <std::size_t FirstDimension, std::size_t... DimN> struct next_dimension
        {
            static const size_type value = next_dimension_secondary<DimN...>::value;
        };

        template <std::size_t... DimN> struct next_dimension_secondary
        {
            static const size_type value = first_dimension<DimN...>::value;
        };

    public:
        
        //! Number of dimensions.
        static const size_type num_dimensions = sizeof...(Dimensions);

        //! Number of all elements in the array.
        static const size_type num_elements = array_size<Dimensions...>::value;

        //! Entire storage size (in bytes).
        static const size_type data_size = sizeof(value_type) * num_elements;

        //! Number of elements to the next slice.
        static const size_type stride = next_array_size<Dimensions...>::value;

    private:

        typedef multi_array<T, Dimensions...> this_array_type;

        typedef std::array<value_type, num_elements> storage_type;

        //! Array data storage.
        storage_type data_;

    public:

        typedef typename storage_type::iterator                  iterator;
        typedef typename storage_type::const_iterator            const_iterator;
        typedef typename storage_type::reverse_iterator          reverse_iterator;
        typedef typename storage_type::const_reverse_iterator    const_reverse_iterator;

        multi_array()
        {
            static_assert(sizeof...(Dimensions) > 0, "multi_array must have at least 1 dimension");
        }
        multi_array(const value_type& value)
        {
            fill(value);
        }
        multi_array(const this_array_type& other) :
            data_(other.data_)
        {
        }
        multi_array(const std::initializer_list<value_type>& list)
        {
            std::copy(list.begin(), list.end(), begin());
        }

        pointer data()
        {
            return data_.data();
        }
        const_pointer data() const
        {
            return data_.data();
        }

        //! Returns the total number of elements.
        size_type size() const
        {
            return num_elements;
        }

        bool empty() const
        {
            return data_.empty();
        }

        //! Returns the maximal number of elements (equal to std::numeric_limits<size_type>::max()).
        size_type max_size() const
        {
            return data_.max_size();
        }

        iterator begin()
        {
            return data_.begin();
        }
        const_iterator begin() const
        {
            return data_.begin();
        }

        reverse_iterator rbegin()
        {
            return data_.rbegin();
        }
        const_reverse_iterator rbegin() const
        {
            return data_.rend();
        }

        iterator end()
        {
            return data_.end();
        }
        const_iterator end() const
        {
            return data_.end();
        }

        reverse_iterator rend()
        {
            return data_.rend();
        }
        const_reverse_iterator rend() const
        {
            return data_.rend();
        }

        reference front()
        {
            return data_.front();
        }
        const_reference front() const
        {
            return data_.front();
        }

        reference back()
        {
            return data_.back();
        }
        const_reference back() const
        {
            return data_.back();
        }

        void fill(const value_type& value)
        {
            data_.fill(value);
        }

        void swap(this_array_type& other)
        {
            data_.swap(other.data_);
        }

        /**
        Returns the number of slices for the specifies dimension.
        \param[in] dimension Specifies the dimension index for the requested slices.
        \throws std::out_of_range If 'dimension' is greater then or equal to the number of array dimensions (num_dimensions).
        \see num_dimensions
        \remarks This is the dynamic version of "slices" which is slower than the static version.
        */
        size_type slices(const size_type& dimension) const
        {
            if (dimension >= num_dimensions)
                throw std::out_of_range("multi_array::slices out of range");
            std::array<size_type, num_dimensions> dim_list { Dimensions... };
            return dim_list[dimension];
        }

        /**
        Returns the number of slices for the specifies dimension.
        \tparam DimensionIndex Specifies the dimension index for the requested slices.
        \see num_dimensions
        \remarks This is the static version of "slices" which is faster than the dynamic version.
        */
        template <size_type DimensionIndex> size_type slices() const
        {
            static_assert(DimensionIndex < num_dimensions, "multi_array::slice out of range");
            return detail::select<size_type, DimensionIndex, Dimensions...>::value;
        }

        template <std::size_t CurrentDimension, std::size_t... NextDimensions> class slice
        {
            
            public:
                
                slice<NextDimensions...> operator [] (const size_type& index)
                {
                    return slice<NextDimensions...>(ptr_ + (multi_array<T, NextDimensions...>::stride * index));
                }

                slice<NextDimensions...> at(const size_type& index)
                {
                    if (index >= first_dimension<NextDimensions...>::value)
                        throw std::out_of_range("multi_array::slice out of range");
                    return slice<NextDimensions...>(ptr_ + (multi_array<T, NextDimensions...>::stride * index));
                }

                slice<CurrentDimension, NextDimensions...>& operator = (const value_type& value)
                {
                    std::fill(ptr_, ptr_ + multi_array<T, CurrentDimension, NextDimensions...>::stride, value);
                    return *this;
                }

            private:
                
                friend class multi_array;

                slice(pointer ptr) : ptr_(ptr)
                {
                }

                pointer ptr_ = nullptr;
                
        };

        template <std::size_t Dimension1, std::size_t Dimension2> class slice<Dimension1, Dimension2>
        {
            
            public:
                
                reference operator [] (const size_type& index)
                {
                    return ptr_[index];
                }

                reference at(const size_type& index)
                {
                    if (index >= Dimension2)
                        throw std::out_of_range("multi_array::slice out of range");
                    return ptr_[index];
                }

                slice<Dimension1, Dimension2>& operator = (const value_type& value)
                {
                    std::fill(ptr_, ptr_ + multi_array<T, Dimension1, Dimension2>::stride, value);
                    return *this;
                }

            private:
                
                friend class multi_array;

                slice(pointer ptr) : ptr_(ptr)
                {
                }

                pointer ptr_ = nullptr;
                
        };

        template <std::size_t CurrentDimension, std::size_t... NextDimensions> class const_slice
        {
            
            public:
                
                const_slice<NextDimensions...> operator [] (const size_type& index) const
                {
                    return const_slice<NextDimensions...>(ptr_ + (multi_array<T, NextDimensions...>::stride * index));
                }

                const_slice<NextDimensions...> at(const size_type& index) const
                {
                    if (index >= first_dimension<NextDimensions...>::value)
                        throw std::out_of_range("multi_array::const_slice out of range");
                    return const_slice<NextDimensions...>(ptr_ + (multi_array<T, NextDimensions...>::stride * index));
                }

            private:
                
                friend class multi_array;

                const_slice(const_pointer ptr) : ptr_(ptr)
                {
                }

                const_pointer ptr_ = nullptr;
                
        };

        template <std::size_t Dimension1, std::size_t Dimension2> class const_slice<Dimension1, Dimension2>
        {
            
            public:
                
                const_reference operator [] (const size_type& index) const
                {
                    return ptr_[index];
                }

                const_reference at(const size_type& index) const
                {
                    if (index >= Dimension2)
                        throw std::out_of_range("multi_array::const_slice out of range");
                    return ptr_[index];
                }

            private:
                
                friend class multi_array;

                const_slice(const_pointer ptr) : ptr_(ptr)
                {
                }

                const_pointer ptr_ = nullptr;
                
        };

        slice<Dimensions...> operator [] (const size_type& index)
        {
            return slice<Dimensions...>(data_.data() + (stride*index));
        }

        const_slice<Dimensions...> operator [] (const size_type& index) const
        {
            return const_slice<Dimensions...>(data_.data() + (stride*index));
        }

        slice<Dimensions...> at(const size_type& index)
        {
            if (index >= first_dimension<Dimensions...>::value)
                throw std::out_of_range("multi_array::slice out of range");
            return slice<Dimensions...>(data_.data() + (stride*index));
        }

        const_slice<Dimensions...> at(const size_type& index) const
        {
            if (index >= first_dimension<Dimensions...>::value)
                throw std::out_of_range("multi_array::slice out of range");
            return const_slice<Dimensions...>(data_.data() + (stride*index));
        }

};


/**
Multi dimensional array class.
\tparam T Specifies the data type for the array elements.
\remarks This is a template specialization of the multi-dimensional multi_array class.
*/
template <class T, std::size_t Dimension> class multi_array<T, Dimension>
{
    
    public:

        typedef T                   value_type;
        typedef std::size_t         size_type;
        typedef std::ptrdiff_t      difference_type;
        typedef value_type&         reference;
        typedef const value_type&   const_reference;
        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;

    public:
        
        //! Number of dimensions.
        static const size_type num_dimensions = 1;

        //! Number of all elements in the array.
        static const size_type num_elements = Dimension;

        //! Entire storage size (in bytes).
        static const size_type data_size = sizeof(value_type) * num_elements;

        //! Number of elements to the next slice.
        static const size_type stride = 1;

    private:

        typedef multi_array<T, Dimension> this_array_type;

        typedef std::array<value_type, num_elements> storage_type;

        //! Array data storage.
        storage_type data_;

    public:

        typedef typename storage_type::iterator                  iterator;
        typedef typename storage_type::const_iterator            const_iterator;
        typedef typename storage_type::reverse_iterator          reverse_iterator;
        typedef typename storage_type::const_reverse_iterator    const_reverse_iterator;

        multi_array()
        {
        }
        multi_array(const value_type& value)
        {
            fill(value);
        }
        multi_array(const this_array_type& other) :
            data_(other.data_)
        {
        }
        multi_array(const std::initializer_list<value_type>& list)
        {
            std::copy(list.begin(), list.end(), begin());
        }

        pointer data()
        {
            return data_.data();
        }
        const_pointer data() const
        {
            return data_.data();
        }

        //! Returns the total number of elements.
        size_type size() const
        {
            return num_elements;
        }

        //! Returns the maximal number of elements (equal to std::numeric_limits<size_type>::max()).
        size_type max_size() const
        {
            return data_.max_size();
        }

        bool empty() const
        {
            return data_.empty();
        }

        iterator begin()
        {
            return data_.begin();
        }
        const_iterator begin() const
        {
            return data_.begin();
        }

        reverse_iterator rbegin()
        {
            return data_.rbegin();
        }
        const_reverse_iterator rbegin() const
        {
            return data_.rbegin();
        }

        iterator end()
        {
            return data_.end();
        }
        const_iterator end() const
        {
            return data_.end();
        }

        reverse_iterator rend()
        {
            return data_.rend();
        }
        const_reverse_iterator rend() const
        {
            return data_.rend();
        }

        reference front()
        {
            return data_.front();
        }
        const_reference front() const
        {
            return data_.front();
        }

        reference back()
        {
            return data_.back();
        }
        const_reference back() const
        {
            return data_.back();
        }

        void fill(const value_type& value)
        {
            data_.fill(value);
        }

        void swap(this_array_type& other)
        {
            data_.swap(other.data_);
        }

        /**
        Returns the number of slices for the specifies dimension.
        \param[in] dimension Specifies the dimension index for the requested slices.
        \throws std::out_of_range If 'dimension' is greater then or equal to the number of array dimensions (num_dimensions).
        \see num_dimensions
        \remarks This is the dynamic version of "slices" which is slower than the static version.
        */
        size_type slices(const size_type& dimension) const
        {
            if (dimension >= num_dimensions)
                throw std::out_of_range("multi_array::slices out of range");
            return Dimension;
        }

        /**
        Returns the number of slices for the specifies dimension.
        \tparam DimensionIndex Specifies the dimension index for the requested slices.
        \see num_dimensions
        \remarks This is the static version of "slices" which is faster than the dynamic version.
        */
        template <size_type DimensionIndex> size_type slices() const
        {
            static_assert(DimensionIndex < num_dimensions, "multi_array::slice out of range");
            return Dimension;
        }

        reference operator [] (const size_type& index)
        {
            return data_[index];
        }

        const_reference operator [] (const size_type& index) const
        {
            return data_[index];
        }

        reference at(const size_type& index)
        {
            return data_.at(index);
        }

        const_reference at(const size_type& index) const
        {
            return data_.at(index);
        }

};


} // /namespace ext


#endif


