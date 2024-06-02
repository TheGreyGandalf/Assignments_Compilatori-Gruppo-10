#clang -O0 -emit-llvm -S -c ciclo.cpp -o Fusion.ll
#rimuovere la parte symbols da quello prodotto nella parte precedente
opt -p LoopFusionPass Fusion.ll -o Fusion.bc
llvm-dis Fusion.bc -o Fusion-finale.ll
