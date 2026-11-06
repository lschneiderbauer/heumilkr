#ifndef DISTMAT_H
#define DISTMAT_H

#include <vector>

template <class T>
class Symmat
{
public:
  Symmat(const std::vector<T> &vec);
  T get(const int i, const int j) const;
  int size() const;

private:
  std::vector<T> data;
  int m_size; // m_size * m_size elements in matrix
};

typedef Symmat<double> Distmat;

#endif
