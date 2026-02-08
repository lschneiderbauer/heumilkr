#include "symmat.h"
#include <cmath>
#include <stdexcept>

template <class T>
Symmat<T>::Symmat(int size, T def)
{
  m_size = size;
  data = std::vector<T>(size * (size - 1) / 2, def);
}

template <class T>
Symmat<T>::Symmat(const std::vector<T> &vec)
{
  m_size = std::round((1 + sqrt(1 + 8 * vec.size())) / 2);
  data = vec;
}

template <class T>
T &Symmat<T>::acc(const int i, const int j)
{
  // for i < j < n we have
  // n * i - i*(i+1)/2 + (j-i) - 1

  if (i >= m_size)
  {
    throw std::out_of_range("Index i out of range in distmat::acc");
  }
  if (j >= m_size)
  {
    throw std::out_of_range("Index j out of range in distmat::acc");
  }

  if (i < j)
  {
    return data[m_size * i - i * (i + 1) / 2 + (j - i) - 1];
  }
  else
  {
    return data[m_size * j - j * (j + 1) / 2 + (i - j) - 1];
  }
}

template <class T>
T Symmat<T>::get(const int i, const int j) const
{
  if (i >= m_size)
  {
    throw std::out_of_range("Index i out of range in distmat::get");
  }
  if (j >= m_size)
  {
    throw std::out_of_range("Index j out of range in distmat::get");
  }

  if (i < j)
  {
    return data[m_size * i - i * (i + 1) / 2 + (j - i) - 1];
  }
  else
  {
    return data[m_size * j - j * (j + 1) / 2 + (i - j) - 1];
  }
}

template <class T>
int Symmat<T>::size() const
{
  return m_size;
}

template <class T>
Symmat<T> Symmat<T>::sub(std::vector<int> &subvec) const
{
  Symmat<T> submat(subvec.size(), 0);

  for (size_t i = 0; i < subvec.size(); i++)
  {
    for (size_t j = 0; j < i; j++)
    {
      submat.acc(i, j) = this->get(subvec[i], subvec[j]);
    }
  }

  return submat;
}

template class Symmat<double>;
