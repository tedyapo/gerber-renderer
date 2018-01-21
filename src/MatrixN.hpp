#ifndef MATRIXN_HPP_INCLUDED_
#define MATRIXN_HPP_INCLUDED_

#include "VectorN.hpp"
#include <util.h>

template <typename base_t, unsigned N>
struct MatrixN
{
  Matrix3(const Matrix3 &a){
    for(int i=0; i<9; i++) m[i] = a.m[i];
  }

  Matrix3& operator=(const Matrix3 &a){
    for(int i=0; i<9; i++) m[i] = a.m[i];
    return *this; 
  }

  Type &operator()(int r, int c){return m[r*3 + c];};
  Type operator()(int r, int c) const {return m[r*3 + c];};

  Matrix3(){for(int i=0; i<9; i++) m[i] = Type(0);}

  Matrix3(Type *v){
    for(int i=0; i<9; i++) m[i] = v[i];
  }

  Matrix3(Type a, Type b, Type c,
          Type d, Type e, Type f,
          Type g, Type h, Type i){
    m[0] = a; m[1] = b; m[2] = c;
    m[3] = d; m[4] = e; m[5] = f;
    m[6] = g; m[7] = h; m[8] = i;
  }

  void read(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      FATAL_ERROR("cannot open %s", filename);
    }
    this->read(fp);
    fclose(fp);
  }

  void read(FILE *fp){
    double val;
    // note everything read as doubles, then converted
    for (int i=0; i<9; i++){
      fscanf(fp, "%lf", &val);
      m[i] = Type(val);
    }
  }

  void write(const char *filename){
    FILE *fp = fopen(filename, "wt");
    if (NULL == fp){
      FATAL_ERROR("cannot open %s", filename);
    }
    this->write(fp);
    fclose(fp);
  }

  void write(FILE *fp){
    double val;
    // note everything converted, then written as doubles
    for (int i=0; i<3; i++){
      for (int j=0; j<3; j++){
        val = double(m[i*3+j]);
        fprintf(fp, "%g ", val);
      }
      fprintf(fp, "\n");
    }
  }

  static  Matrix3<Type> identity(){
    return Matrix3<Type>(Type(1), Type(0), Type(0),
                         Type(0), Type(1), Type(0),
                         Type(0), Type(0), Type(1));
  }

  Matrix3<Type>& operator*(Type a){
    for (int i=0; i<9; i++){
      m[i] *= a;
    }
    return *this;
  }

  Matrix3<Type> operator*(const Matrix3<Type> &a) const{
    Matrix3<double> temp;
    for (int i=0; i<3; i++){
      for (int j=0; j<3; j++){
        temp(i,j) = Type(0);
        for (int k=0; k<3; k++){
          temp(i,j) += (*this)(i,k) * a(k,j);
        }
      }
    }
    //    for (int i=0; i<9; i++){
    //  m[i] = temp.m[i];
    //}
    //return *this;
    return temp;
  }

  Matrix3<Type> operator+(const Matrix3<Type> &a) const {
    Matrix3<double> mat = *this;
    for (int i=0; i<9; i++){
      mat.m[i] += a.m[i];
    }
    return mat;
  }

  Matrix3<Type> &operator+=(const Matrix3<Type> &a) {
    for (int i=0; i<9; i++){
      m[i] += a.m[i];
    }
    return *this;
  }


  Matrix3<Type> inverse() const {
    Type det = ((*this)(0,0) * ((*this)(2,2)*(*this)(1,1) - 
                                (*this)(2,1)*(*this)(1,2)) -
                (*this)(1,0) * ((*this)(2,2)*(*this)(0,1) - 
                                (*this)(2,1)*(*this)(0,2)) +
                (*this)(2,0) * ((*this)(1,2)*(*this)(0,1) - 
                                (*this)(1,1)*(*this)(0,2)));
    
    Matrix3<Type> mat((*this)(2,2)*(*this)(1,1) - (*this)(2,1)*(*this)(1,2),
                      -((*this)(2,2)*(*this)(0,1) - (*this)(2,1)*(*this)(0,2)),
                      (*this)(1,2)*(*this)(0,1) - (*this)(1,1)*(*this)(0,2),
                      -((*this)(2,2)*(*this)(1,0) - (*this)(2,0)*(*this)(1,2)),
                      (*this)(2,2)*(*this)(0,0) - (*this)(2,0)*(*this)(0,2),
                      -((*this)(1,2)*(*this)(0,0) - (*this)(1,0)*(*this)(0,2)),
                      (*this)(2,1)*(*this)(1,0) - (*this)(2,0)*(*this)(1,1),
                      -((*this)(2,1)*(*this)(0,0) - (*this)(2,0)*(*this)(0,1)),
                      (*this)(1,1)*(*this)(0,0) - (*this)(1,0)*(*this)(0,1));
    mat = mat * (1./det);
    return mat;
  }

  Matrix3<Type> transpose() const {
    Matrix3<Type> mat;
    for (int r=0; r<3; r++){
      for (int c=0; c<3; c++){
	mat(r, c) = (*this)(c, r);
      }
    }
    return mat;
  }

  Vector3<Type> operator*(const Vector3<Type> &x) const {
    Vector3<Type> temp;
    temp(0) = m[0]*x.v[0] + m[1]*x.v[1] + m[2]*x.v[2];
    temp(1) = m[3]*x.v[0] + m[4]*x.v[1] + m[5]*x.v[2];
    temp(2) = m[6]*x.v[0] + m[7]*x.v[1] + m[8]*x.v[2];
    return temp;
  }

  void dump(){
    for (int i=0; i<3; i++){
      for (int j=0; j<3; j++){
        printf(" %18.10f ", double((*this)(i,j)));
      }
      printf("\n");
    }
  }
private:
  Type m[9];
};

typedef Matrix3<double> m3d;

template <typename Type>
Matrix3<Type> outer_product(const Vector3<Type> &a,
			    const Vector3<Type> &b){
  Matrix3<Type> mat;
  for (int r=0; r<3; r++){
    for (int c=0; c<3; c++){
      mat(r,c) = a(r) * b(c);
    }
  }
  return mat;
}

#endif // #ifndef MATRIXN_HPP_INCLUDED_
