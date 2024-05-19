/*
 * Codice per testare le chains delle funzioni, 2024-04-22
 * */

#ifndef LLVM_TRANSFORMS_LOOPPASSES_H
#define LLVM_TRANSFORMS_LOOPPASSES_H
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"

namespace llvm {
    class LoopPassEs : public PassInfoMixin<LoopPassEs>{
        PreservedAnalyses run(Loop &L, LoopAnalysisManager &LAM, LoopStandardAnalysisResults &LAR, LPMUpdater &LU);

    }; //namespace llvm
}
#endif // LLVM_TRANSFORMS_LOOPPASSES_H
