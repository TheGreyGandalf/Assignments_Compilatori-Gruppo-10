#include <iostream>
using namespace std;

void twoloops(int a[],int b[],int c[],int d[], int n){

  int i;
  for (i=0; i<n; i++) {
    a[i] = b[i] + c[i];
  }

  for (i=0; i<n; i++) {
    d[i] = a[i] * a[i];
  }
}

int main(){
  int a[100],b[100],c[100],d[100];
  int n=100;

  twoloops(a,b,c,d,n);

  return 0;
}
