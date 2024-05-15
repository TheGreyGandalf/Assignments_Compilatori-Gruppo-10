#clang -O0 -emit-llvm -S -c LICM-passo.c -o LICM-passo.ll
#rimuovere la parte symbols
opt -p LICM LICM-passo.ll -o LICM-passo.bc
llvm-dis LICM-passo.bc -o LICM-finale.ll
