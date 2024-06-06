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

  // Raccogli le istruzioni nel primo loop
  std::vector<Instruction *> FirstLoopInstructions;
  for (BasicBlock *BB : First->blocks()) {
    for (Instruction &I : *BB) {
      FirstLoopInstructions.push_back(&I);
    }
  }

  // Raccogli le istruzioni nel secondo loop
  std::vector<Instruction *> SecondLoopInstructions;
  for (BasicBlock *BB : Second->blocks()) {
    for (Instruction &I : *BB) {
      SecondLoopInstructions.push_back(&I);
    }
  }

  // Controlla le dipendenze tra le istruzioni dei due loop
  for (Instruction *I1 : FirstLoopInstructions) {
    for (Instruction *I2 : SecondLoopInstructions) {
      std::unique_ptr<Dependence> Dep = DI.depends(I1, I2, true);
      if (Dep && !Dep->isLoopIndependent()) {
        // Se esiste una dipendenza che impedisce la fusione, ritorna false
        outs() << "Esiste una dipendenza tra le istruzioni: ";
        I1->print(outs());
        outs() << " e ";
        I2->print(outs());
        outs() << "\n";
        return false;
      }
    }
  }

  // Se non ci sono dipendenze che impediscono la fusione, ritorna true
  return true;
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

  //for (auto iteraz = LI.end() - 1; iteraz != LI.begin(); --iteraz) {
  for (auto iteraz = LI.begin(); iteraz != LI.end(); iteraz++) {
    //Ciclo da prendere con le pinze, non sono sicuro

    Loop *Primo = *iteraz;
    auto nextIteraz = std::next(iteraz);

    if (nextIteraz == LI.end())
      break;

    //if (iteraz == LI.begin())
    //  break;

    Loop *Secondo = *nextIteraz;
    //*(iteraz + 1);
    
    outs() << "Ora effettuiamo i controlli\n";

    bool primo = Controllo_1(Primo, Secondo);
    bool secondo = Controllo_2(Primo, Secondo, SC);
    bool terzo = Controllo_3(Primo, Secondo, DT, PDT);
    bool quarto = Controllo_4(Primo, Secondo, DI);

    if (primo && secondo && terzo && quarto) {
      outs() << "I due loop sono adiacenti\n";
    }
    else {
      outs() << "Due cicli non adiacenti\n";
    }
  }

  return PreservedAnalyses::all();
}
