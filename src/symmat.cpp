#include "symmat.h"
#include <stdint.h>

template <class T>
symmat<T>::symmat(int n, T def)
{
  size = n;
  data.reserve(n);

  for (int i = 0; i < n; i++)
  {
    std::vector<T> row(n - i, def);
    data.push_back(row);
  }
}

template <class T>
symmat<T>::symmat(std::vector<T> vec, int n)
{
  size = n;
  data.reserve(n);

  for (int i = 0; i < n; i++)
  {
    std::vector<T> row(n - i);
    for (int j = 0; j < n - i; j++)
    {
      row[j] = vec[i * n + j];
    }
    data.push_back(row);
  }
}

template <class T>
T &symmat<T>::acc(const int row, const int col)
{
  int r = row;
  int c = col;
  if (col < row)
  {
    r = col;
    c = row;
  }
  return data[r][c - r];
}

template <class T>
T symmat<T>::get(const int row, const int col) const
{
  int r = row;
  int c = col;
  if (col < row)
  {
    r = col;
    c = row;
  }
  return data[r][c - r];
}

template class symmat<double>;
