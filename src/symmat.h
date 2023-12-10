#include <vector>

#ifndef SYMMAT_H
#define SYMMAT_H

template <class T>
class symmat
{
public:
  symmat(int n = 0, T def = T());
  symmat(std::vector<T> vec, int n);
  T &acc(const int row, const int col);
  T get(const int row, const int col) const;
  int size;

private:
  std::vector<std::vector<T>> data;
};

#endif
