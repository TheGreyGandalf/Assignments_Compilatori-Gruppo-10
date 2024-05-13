/*
 *  Codice per estendere quello che una funzione fa
 *  Modificato poi 2024-04-29 per utilizzarlo per i loop
 * */

#include "llvm/Transforms/Utils/Chains.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_set>
using namespace llvm;

bool isOperandLI(const Use &usee, Loop &l, auto list) {
  if (dyn_cast<Constant>(usee))
    return true;
  if (dyn_cast<Argument>(usee))
    return true;

  decltype(auto) i = dyn_cast<Instruction>(usee);

  // Controllo nella lista se l'istruzione è riconosciuta e contenuta nella
  // lista
  if (i and not l.contains(i)) {
    // Se riconosciuta allora viene ritornata come possibile LICM
    return true;
  }
  if (i and list.count(i)) {
    return true;
  }

  return false;
};

bool doesDominateExitPoint(DominatorTree &dominators, Instruction *instr,
                           auto NodiUscita) {
  bool dominate = true;
  for (const auto edge : NodiUscita)
    dominate &= dominators.dominates(instr, edge.second);
  return dominate;
};

PreservedAnalyses Chains::run(Loop &L, LoopAnalysisManager &LAM,
                              LoopStandardAnalysisResults &LAR,
                              LPMUpdater &LU) {

  if (!L.isLoopSimplifyForm()) {
    outs() << "\nLOOP NON IN FORMA NORMALE\n";
    return PreservedAnalyses::all();
  }

  outs() << "\nLOOP IN FORMA NORMALE\n";

  BasicBlock *head = L.getHeader();
  Function *F = head->getParent();

  outs() << "\n***CFG DELLA FUNZIONE:***\n";

  for (auto iter = F->begin(); iter != F->end(); ++iter) {
    BasicBlock &BB = *iter;
    outs() << BB << "\n";
  }

  outs() << "***IL LOOP***\n";
  // Stampa del loop
  for (auto BI = L.block_begin(); BI != L.block_end(); ++BI) {
    BasicBlock *BB = *BI;
    outs() << *BB << "\n";

    outs() << "********\n";
    // Modifiche del codice fatto da me

    /*
    //Parte di Use CHAINS e UD Chains
    // Siamo nel BB
    BasicBlock *interno = *BI;
    for (auto instr = interno->begin(); instr != interno->end(); ++instr) {
      // Siamo nella istruzioe
      Instruction &in = *instr;
      outs() << "Sono l'istruzione" << in << "\n";
      outs() << "LHS:";
      //outs() << in.printAsOperand(outs(), false) << "\n";
    }*/
  }

  auto IstruzioniLoopInv = std::unordered_set<Instruction *>{};

  // Ciclo per estrarre le istruzioni che sono loop-invariant
  // Per tutti i blocchi contenuti nel loop
  for (auto *bb : L.blocks()) {
    // Iterazione sulle istruzioni nei blocchi
    for (auto &i : *bb) {
      bool isInstrLI = true;
      for (const auto &usee : i.operands()) {
        // Passaggio alla funzione che determina se un Use è costante
        // e se sono anche loop invariant
        isInstrLI &= isOperandLI(usee, L, IstruzioniLoopInv);
      }

      // Abbiamo controllato e siamo arrivati alla conclusione che è una
      // Istruzione loop invariant, la inseriamo nella lista che le
      // raggruppa
      if (isInstrLI)
        IstruzioniLoopInv.insert(&i);
    }
  }

  for (auto *i : IstruzioniLoopInv) {
    i->print(errs());
    errs() << "\n";
  }

  // Identificazione dei blocchi uscita dal loop
  Instruction *instr;
  auto NodiUscita = SmallVector<std::pair<BasicBlock *, BasicBlock *>>{};

  L.getExitEdges(NodiUscita);
  // const auto parentFunction = L.getBlocks()[0]->getParent();
  DominatorTree &DT = LAR.DT;
  //Passaggio alla funzione che controlla se le istruzioni dominano tutte le istruzioni
  doesDominateExitPoint(DT, instr, NodiUscita);

  // DominatorTree &DT = LAR.DT;
  // BasicBlock *BB = (DT.getRootNode())->getBlock();

  return PreservedAnalyses::all();
}
