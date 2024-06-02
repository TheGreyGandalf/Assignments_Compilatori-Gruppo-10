#include "llvm/Transforms/Utils/LoopFusionPass.h"
#include "llvm/IR/Dominators.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/GenericDomTree.h>
#include <llvm/Support/raw_ostream.h>

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>

#include <sched.h>
#include <unordered_set>
using namespace llvm;
using namespace std;


bool Guardia(Loop *First, Loop *Second) {
  BasicBlock *B = First->getLoopGuardBranch()->getSuccessor(0);
  // Noi non vogliamo il pre-header, ci interessa il successore
  // non loop
  if (B == First->getLoopPreheader()) {
    B = First->getLoopGuardBranch()->getSuccessor(0);
  }

  if (B == Second->getHeader()) {
    return true;
  }
  return false;
}

bool Controllo_1(Loop *First, Loop *Second) {
  // 1) Primo controllo entrambi devono essere adiacenti

  if (First->getExitBlock() == Second->getLoopPreheader()) {
    outs() << "Non c'è la guardia\n";
    return true;
  } else if (First->isGuarded() && Second->isGuarded()) {
    outs() << "Entrambi con guardia\n";
    if (Guardia(First, Second)) {
      return true;
    }
  }

  return false;
}

bool Controllo_2(Loop *First, Loop *Second, ScalarEvolution &SC) {
  // 2) Secondo controllo entrambi
  // Devono avere lo stesso numero di iterazioni

  const SCEV *PR = SC.getBackedgeTakenCount(First);
  const SCEV *SE = SC.getBackedgeTakenCount(Second);

  if (!isa<SCEVCouldNotCompute>(PR) && !isa<SCEVCouldNotCompute>(SE)) {

    if (PR == SE) {
      return true;
    }
  }

  outs() << "I due loop non hanno lo stesso numero di iterazioni\n";
  return false;
}

bool Controllo_3(Loop *First, Loop *Second, DominatorTree &DT,
                 PostDominatorTree &PDT) {
  // 3) Terzo controllo entrambi devono essere adiacenti
  BasicBlock *FirstStart = First->getHeader();
  BasicBlock *FirstEnd = First->getExitBlock();
  BasicBlock *SecondStart = Second->getHeader();
  BasicBlock *SecondEnd = Second->getExitBlock();

  // Verifica della dominanza
  bool FirstDominatesSecond = DT.dominates(FirstEnd, SecondStart);
  bool SecondPostDominatesFirst = PDT.dominates(SecondEnd, FirstStart);

  // Condizione di adiacenza
  if (FirstDominatesSecond || SecondPostDominatesFirst) {
    return true;
  }

  return false;
}

bool Controllo_4(Loop *First, Loop *Second, DependenceInfo &DI) {
  // 4) Terzo controllo entrambi devono Essere indipendenti

  //auto dep = DI.depends(&I0, &I1, true);
  //if (!DepResult)
    //return false;
}

PreservedAnalyses LoopFusionPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {

  LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
  // Terzo punto
  DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
  PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
  // Secondo punto
  ScalarEvolution &SC = AM.getResult<ScalarEvolutionAnalysis>(F);

  // Quarto punto
  DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

  // Scorrimento dei loop all'interno della Funzione
  for (auto iteraz = LI.end() - 1; iteraz != LI.begin(); iteraz--) {

    Loop *Primo = *iteraz;
    Loop *Secondo = *(iteraz + 1);

    bool primo = Controllo_1(Primo, Secondo);
    bool secondo = Controllo_2(Primo, Secondo, SC);
    bool terzo = Controllo_3(Primo, Secondo, DT, PDT);
    bool quarto = Controllo_4(Primo, Secondo, DI);

    if (primo && secondo && terzo && quarto) {
      outs() << "I due loop sono adiacenti\n";
    }
  }

  return PreservedAnalyses::all();
}
