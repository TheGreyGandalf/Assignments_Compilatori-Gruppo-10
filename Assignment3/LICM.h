/*
 * Codice per testare le chains delle funzioni, 2024-04-22
 * */

#ifndef LLVM_TRANSFORMS_CHAINS_H
#define LLVM_TRANSFORMS_CHAINS_H
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
namespace llvm {
class Chains : public PassInfoMixin<Chains> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &LAM,
                        LoopStandardAnalysisResults &LAR, LPMUpdater &LU);

  // PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace llvm
#endif // LLVM_TRANSFORMS_TESTPASS _H
