#rimuovere la parte symbols da quello prodotto nella parte precedente
opt -p LoopFusionPass Fusion-passo.ll -o Fusion.bc
llvm-dis Fusion.bc -o Fusion-finale.ll
