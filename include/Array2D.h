//
// Created by AJ Pryor on 1/27/17
//

#ifndef PRISM_ARRAY2D_H
#define PRISM_ARRAY2D_H
#include <vector>
#include <cstddef>

namespace PRISM {

    template <class T>
    class Array2D {
    public:
        Array2D(T _data,
                const size_t& _nrows,
                const size_t& _ncols);
        Array2D(){};
        size_t get_nrows() const {return this->nrows;}
        size_t get_ncols() const {return this->ncols;}
        size_t size()      const {return this->N;}
        typename T::iterator begin();
        typename T::iterator end();
        typename T::value_type& at(const size_t& i, const size_t& j);

    private:
        T data;
        size_t nrows;
        size_t ncols;
        size_t N;
    };

    template <class T>
    Array2D<T>::Array2D(T _data,
                        const size_t& _nrows,
                        const size_t& _ncols):data(_data), nrows(_nrows), ncols(_ncols){
        if (_data.size() != (_nrows * _ncols)) throw "PRISM: Size mismatch. Array size does not equal nrows * ncols";
        this->N = _nrows * _ncols;
    };

    template <class T>
    typename T::iterator Array2D<T>::begin(){return this->data.begin();}

    template <class T>
    typename T::iterator Array2D<T>::end(){return this->data.end();}

    template <class T>
    typename T::value_type& Array2D<T>::at(const size_t& i, const size_t& j){
        return data[i*ncols + j];
    }
}


#endif //PRISM_ARRAY2D_H
