/*
 *  Codice per estendere quello che una funzione fa
 *  Modificato poi 2024-04-29 per utilizzarlo per i loop
 * */

#include "llvm/Transforms/Utils/LoopPassEs.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_set>
using namespace llvm;


bool isOperandLI(const Use &usee, const Loop &l, const std::unordered_set<Instruction *> &invariantInstructions) {
  if (isa<Constant>(usee) || isa<Argument>(usee))
    return true;

  if (auto *inst = dyn_cast<Instruction>(usee)) {
    if (!l.contains(inst) || invariantInstructions.count(inst)) {
      return true;
    }
  }
  return false;
}

bool doesDominateExitPoints(const DominatorTree &DT, const Instruction *instr, const SmallVectorImpl<std::pair<BasicBlock *, BasicBlock *>> &exitEdges) {
  for (const auto &edge : exitEdges) {
    if (!DT.dominates(instr->getParent(), edge.second)) {
      return false;
    }
  }
  return true;
}

void findLoopInvariantInstructions(Loop &L, std::unordered_set<Instruction *> &invariantInstructions, LoopStandardAnalysisResults &LAR) {
  for (BasicBlock *BB : L.blocks()) {
    for (Instruction &I : *BB) {
      bool isInvariant = true;
      for (const Use &usee : I.operands()) {
        if (!isOperandLI(usee, L, invariantInstructions)) {
          isInvariant = false;
          break;
        }
      }
      if (isInvariant) {
        invariantInstructions.insert(&I);
      }
    }
  }

  SmallVector<std::pair<BasicBlock *, BasicBlock *>, 4> exitEdges;
  L.getExitEdges(exitEdges);

  for (auto it = invariantInstructions.begin(); it != invariantInstructions.end(); ) {
    if (!doesDominateExitPoints(LAR.DT, *it, exitEdges)) {
      it = invariantInstructions.erase(it);
    } else {
      ++it;
    }
  }
}

PreservedAnalyses LoopPassEs::run(Loop &L, LoopAnalysisManager &LAM, LoopStandardAnalysisResults &LAR, LPMUpdater &LU) {
  if (!L.isLoopSimplifyForm()) {
    outs() << "\nLOOP NON IN FORMA NORMALE\n";
    return PreservedAnalyses::all();
  }

  outs() << "\nLOOP IN FORMA NORMALE\n";

  BasicBlock *preHeader = L.getLoopPreheader();
  if (!preHeader) {
    outs() << "Preheader non trovato\n";
    return PreservedAnalyses::all();
  }

  std::unordered_set<Instruction *> invariantInstructions;
  findLoopInvariantInstructions(L, invariantInstructions, LAR);

  int ind = 1;
  errs() << "\nStampa delle istruzioni che sono loop invariant\n";
  for (Instruction *I : invariantInstructions) {
    errs() << "Istruzione numero " << ind << ": ";
    I->print(errs());
    errs() << "\n";
    ind++;
  }

  for (Instruction *I : invariantInstructions) {
    I->moveBefore(preHeader->getTerminator());
  }

  return PreservedAnalyses::all();
}
