#ifndef DISTMAT_H
#define DISTMAT_H

#include <vector>

template <class T>
class distmat
{
public:
  distmat(int n = 0, T def = T());
  distmat(const std::vector<T> &vec);
  T &acc(const int i, const int j);
  T get(const int i, const int j) const;
  int size() const;
  distmat<T> sub(std::vector<int> &subvec) const;

private:
  std::vector<T> data;
  int m_size; // m_size * m_size elements in matrix
};

#endif
