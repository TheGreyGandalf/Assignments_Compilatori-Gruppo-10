clang -O0 -emit-llvm -S -c LICM.c -o LICM.ll
#Si deve commentare anche le righe con gli attributes
opt -p mem2reg LICM.ll -o LICM.bc
llvm-dis LICM.bc -o LICM-passo.ll
