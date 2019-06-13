#ifndef SGI_PLUS_H
#define SGI_PLUS_H

#include <vector>
#include <list>
#include <iterator>

namespace std {

/**
 * @brief The SGIList class 对sgi版本的list进行包装，解决size方法导致的性能问题
 * @attention 当前包装没有对resize方法进行处理
 */
template<class T>
class SGIList : public list<T>
{
    typedef typename list<T>::iterator sgi_iterator;
    typedef typename list<T>::size_type sig_size_type;
public:

    SGIList(): num(0) {}
    ~SGIList() {}

    void push_back(const T& _Val)
    {
        list<T>::push_back(_Val);
        num++;
    }

    void push_front(const T& _Val)
    {
        list<T>::push_front(_Val);
        num++;
    }

    void pop_front()
    {
        list<T>::pop_front();
        num--;
    }

    void pop_back()
    {
        list<T>::pop_back();
        num--;
    }
    
    sgi_iterator insert(sgi_iterator _Where, const T& _Val)
    {
        list<T>::insert(_Where,_Val);
        num++;
        return --_Where;
    }

    sgi_iterator erase(sgi_iterator _Where)
    {
        num--;
        return list<T>::erase(_Where);
    }
    
    sgi_iterator erase(sgi_iterator _First, sgi_iterator _Last)
    {
        while (_First != _Last)        
        {            
            _First = this->erase(_First);        
        }        
        return _Last;
    }

    void clear()
    {
        list<T>::clear();
        num = 0;
    }

    sig_size_type size()const
    {
        return num;
    }

    bool empty() const
    {
        return num == 0;
    }

private:
    sig_size_type num;
};

/**
 * @brief The SGIVector class 对sgi版本的vector进行包装，解决size方法导致的性能问题
 * @attention 当前包装没有对resize方法进行处理
 */
template<class T>
class SGIVector : public vector<T>
{
    typedef typename vector<T>::iterator sgi_iterator;
    typedef typename vector<T>::size_type sig_size_type;
public:
public:
    SGIVector(): num(0) {}
    ~SGIVector() {}

    void push_back(const T& _Val)
    {
        vector<T>::push_back(_Val);
        num++;
    }

    void push_front(const T& _Val)
    {
        vector<T>::push_front(_Val);
        num++;
    }

    void pop_front()
    {
        vector<T>::pop_front();
        num--;
    }

    void pop_back()
    {
        vector<T>::pop_back();
        num--;
    }

    sgi_iterator insert(sgi_iterator _Where, const T& _Val)
    {
        vector<T>::insert(_Where,_Val);
        num++;
        return --_Where;
    }

    sgi_iterator erase(sgi_iterator _Where)
    {
        num--;
        return vector<T>::erase(_Where);
    }

    sgi_iterator erase(sgi_iterator _First, sgi_iterator _Last)
    {
        while (_First != _Last)        
        {            
            _First = this->erase(_First);        
        }        
        return _Last;
    }

    void clear()
    {
        vector<T>::clear();
        num=0;
    }

    sig_size_type size()
    {
        return num;
    }

    bool empty() const
    {
        return num == 0;
    }

private:
    sig_size_type num;
};

}
#endif // SGI_PLUS_H

