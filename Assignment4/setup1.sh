#Si applica sempre il mem2reg per poter utilizzare i registri phi
clang -O0 -emit-llvm -S -c ciclo.cpp -o Fusion.ll
#Si deve commentare anche le righe con gli attributes
opt -p mem2reg Fusion.ll -o Fusion.bc
llvm-dis Fusion.bc -o Fusion-passo.ll
