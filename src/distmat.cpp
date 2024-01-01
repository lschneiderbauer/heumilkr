#include "distmat.h"
#include <cmath>

template <class T>
distmat<T>::distmat(int size, T def)
{
  m_size = size;
  data = std::vector<T>(size * (size - 1) / 2, def);
}

template <class T>
distmat<T>::distmat(const std::vector<T> &vec)
{
  m_size = std::round((1 + sqrt(1 + 8 * vec.size())) / 2);
  data = vec;
}

template <class T>
T &distmat<T>::acc(const int i, const int j)
{
  // for i < j < n we have
  // n * i - i*(i+1)/2 + (j-i) - 1

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
T distmat<T>::get(const int i, const int j) const
{
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
int distmat<T>::size() const
{
  return m_size;
}

template class distmat<double>;
