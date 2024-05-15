/*
 *  Codice per estendere quello che una funzione fa
 *  Modificato poi 2024-04-29 per utilizzarlo per i loop
 * */

#include "llvm/Transforms/Utils/Chains.h"
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
  // Scorro tutti i nodi di uscita
  for (const auto edge : NodiUscita)
    // Controllo se il dominator controlla l'istruzione
    dominate &= dominators.dominates(instr, edge.second);
  // Ritorno true e false
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

  int ind = 1;
  errs() << "\nStampa delle istruzioni che sono loop invariant\n";
  for (auto *i : IstruzioniLoopInv) {
    errs() << "Istruzione numero " << ind << ": ";
    i->print(errs());
    errs() << "\n";
    ind++;
  }

  // Ora arriva la seconda parte, scorriamo l'unordered set che abbiamo creato
  // Per poi controllare se le istruzioni immagazzinate sono
  // Dominant, se lo sono è una istruzione candicata ad essere spostata
  // nel pre-header

  // Iterazione sugli elementi del set riempito prima
  for (Instruction *instr : IstruzioniLoopInv) {

    // Identificazione dei blocchi uscita dal loop
    auto NodiUscita = SmallVector<std::pair<BasicBlock *, BasicBlock *>>{};

    // Si inserisce all'interno dello SmallVector gli edge di uscita del Loop
    L.getExitEdges(NodiUscita);
    // Si trovano quelli che sono i Dominator Tree
    DominatorTree &DT = LAR.DT;
    // Passaggio alla funzione che controlla se le istruzioni dominano tutte le
    // istruzioni
    bool SeDomina = doesDominateExitPoint(DT, instr, NodiUscita);
    // se questa variabile è vera significa che domina tutte le sue definzioni
    // future Quindi si devono utilizzare i phinode

    /*PARTE PHI NODE*/
    if (SeDomina) {
      BasicBlock *preHeader = L.getLoopPreheader();
      // Qua sposto l'istruzione al pre headers
      BasicBlock *Header = L.getHeader();
      // La remove lo "sgancia" dalla lista padre, per eliminare si usa erase
      // instr->removeFromParent();
      //preHeader->getInstructionList(instr);
    }
  }

  // Devo prima usare un set per cercare tutte le istruzioni che servono per
  // essere rimosse Poi controlli se sono dominanti Se lo sono iteri nella
  // istruzione e la sposti

  return PreservedAnalyses::all();
}
