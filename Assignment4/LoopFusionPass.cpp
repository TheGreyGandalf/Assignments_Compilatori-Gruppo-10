#include "llvm/Transforms/Utils/LoopFusionPass.h"

//#include "llvm/IR.Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/LLVMContext.h>

#include <llvm/Support/Casting.h>
#include <llvm/Support/GenericDomTree.h>
#include <llvm/Support/raw_ostream.h>

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/PostDominators.h"
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/ScalarEvolution.h>

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <sched.h>
using namespace llvm;
using namespace std;


bool Guardia(Loop *First, Loop *Second) {
  BranchInst *GB = First->getLoopGuardBranch();
  for (unsigned i = 0; i < GB->getNumSuccessors(); i++) {
    BasicBlock *B = GB->getSuccessor(i);
    if (B == Second->getHeader()) {
      return true;
    }
  }
  return false;
}

bool Controllo_1(Loop *First, Loop *Second) {
  // 1) Primo controllo entrambi devono essere adiacenti

  if (First->getExitBlock() == Second->getLoopPreheader()
      || First->getExitBlock() == Second->getHeader()) {
    outs() << "1) Non c'è la guardia e sono adiecenti\n";
    return true;
  } else if (First->isGuarded() && Second->isGuarded()) {
    outs() << "1) Entrambi con guardia\n";
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

  // if (!isa<SCEVCouldNotCompute>(PR) && !isa<SCEVCouldNotCompute>(SE)) {

  if (PR == SE) {
    outs() << "2) Entrambi con lo stessoo numero di istruzioni\n";
    return true;
  }
  //}

  outs() << "I due loop non hanno lo stesso numero di iterazioni\n";
  return false;
}

bool Controllo_3(Loop *First, Loop *Second, DominatorTree &DT,
                 PostDominatorTree &PDT) {
  // 3) Terzo controllo entrambi devono essere adiacenti
  BasicBlock *FirstStart = First->getLoopPreheader();
  BasicBlock *FirstEnd = First->getExitBlock();
  BasicBlock *SecondStart = Second->getLoopPreheader();
  BasicBlock *SecondEnd = Second->getExitBlock();

  // Verifica della dominanza
  bool FirstDominatesSecond = DT.dominates(FirstEnd, SecondStart);
  bool SecondPostDominatesFirst = PDT.dominates(SecondEnd, FirstStart);

  // Condizione di adiacenza
  if (FirstDominatesSecond || SecondPostDominatesFirst) {
    outs() << "3) Il Primo domina il secondo\n";
    return true;
  }

  return false;
}

bool Controllo_4(Loop *First, Loop *Second, DependenceInfo &DI) {
  // 4) Terzo controllo entrambi devono Essere indipendenti

  // Raccogli le istruzioni nel Primo loop
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

  outs() << "4) Indipendenza di entrambi verificata\n";

  // Se non ci sono dipendenze che impediscono la fusione, ritorna true
  return true;
}

// Funzione che ha il compito di ottimizzare i due loop che hanno superato tutti
// e 4 i check
Loop *Ottimizza(Loop *Primo, Loop *Secondo, Function &F,
                FunctionAnalysisManager &AM, LoopInfo &LI,
                ScalarEvolution &SC) {

  // Ottenimento di tutti i dati del primo Loop
  auto PrimoPreheader = Primo->getLoopPreheader();
  auto PrimoLatch = Primo->getLoopLatch();
  // In questi due era diverso ma dava errore senno
  BasicBlock *PrimoBody /* =Primo->getSinglePredecessor()*/;

  for (BasicBlock *pred : predecessors(PrimoLatch)) {
    if (Primo->contains(pred)) {
      PrimoBody = pred;
      break;
    }
  }
  BasicBlock *PrimoBodyEntry /*= Primo->getSingleSuccessor()*/;

  for (BasicBlock *succ : predecessors(PrimoPreheader)) {
    if (Primo->contains(succ)) {
      PrimoBodyEntry = succ;
      break;
    }
  }
  auto PrimoGuard = Primo->getLoopGuardBranch();

  // Ottenimento di tutti i dati del secondo Loop
  auto SecondoPreheader = Secondo->getLoopPreheader();
  auto SecondoLatch = Secondo->getLoopLatch();
  BasicBlock *SecondoBody /*= Secondo->getSinglePredecessor()*/;

  for (BasicBlock *pred : predecessors(SecondoLatch)) {
    if (Primo->contains(pred)) {
      SecondoBody = pred;
      break;
    }
  }
  BasicBlock *SecondoBodyEntry /*= Secondo->getSingleSuccessor()*/;

  for (BasicBlock *succ : predecessors(SecondoPreheader)) {
    if (Primo->contains(succ)) {
      SecondoBodyEntry = succ;
      break;
    }
  }
  auto SecondoExit = Secondo->getExitBlock();

  // Recupera blocchi del secondo Loop
  SmallVector<BasicBlock *, 8> Aggiungi(Secondo->blocks());
  Aggiungi.erase(remove(Aggiungi.begin(), Aggiungi.end(), SecondoLatch),
                 Aggiungi.end());
  Aggiungi.push_back(SecondoPreheader);

  // Aggiornamento variabili di induzione
  PHINode *PrimoIV = Primo->getInductionVariable(SC);
  Value *PrimoValueIV = dyn_cast<Value>(PrimoIV);

  PHINode *SecondoIV = Secondo->getInductionVariable(SC);
  Value *SecondoValueIV = dyn_cast<Value>(SecondoIV);

  SecondoValueIV->replaceAllUsesWith(PrimoValueIV);
  SecondoIV->eraseFromParent();

  // Sposta Phi nodes dal secondo Loop al primo Loop
  for (PHINode &phi : SecondoBodyEntry->phis()) {
    phi.replaceUsesOfWith(SecondoLatch, PrimoLatch);
    phi.replaceUsesOfWith(SecondoPreheader, PrimoPreheader);
  }
  SecondoExit->replacePhiUsesWith(SecondoLatch, PrimoLatch);

  SmallVector<Instruction *, 8> toBeMoved;
  for (Instruction &SecondoInst : *SecondoBodyEntry) {
    if (isa<PHINode>(SecondoInst)) {
      toBeMoved.push_back(&SecondoInst);
    }
  }

  Instruction *movePoint = PrimoBodyEntry->getFirstNonPHI();
  for (Instruction *i : toBeMoved) {
    i->moveBefore(movePoint);
  }

  // Modifica il CFG per riflettere la fusione
  PrimoLatch->getTerminator()->setSuccessor(1, SecondoExit);
  PrimoBody->getTerminator()->replaceSuccessorWith(PrimoLatch,
                                                   SecondoPreheader);
  SecondoBody->getTerminator()->replaceSuccessorWith(SecondoLatch, PrimoLatch);
  SecondoLatch->getTerminator()->replaceSuccessorWith(SecondoExit,
                                                      SecondoLatch);
  if (PrimoGuard)
    PrimoGuard->setSuccessor(1, SecondoExit);

  // Pulizia dei blocchi irraggiungibili
  EliminateUnreachableBlocks(F);

  // Elimina il secondo Loop e aggiungi i suoi blocchi al primo Loop
  LI.erase(Secondo);
  for (BasicBlock *bb : Aggiungi) {
    Primo->addBasicBlockToLoop(bb, LI);
  }

  // Restituisci il primo loop del blocco fuso
  return Primo;
}

PreservedAnalyses LoopFusionPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {

  // Primo punto
  LoopInfo &LI = AM.getResult<LoopAnalysis>(F);

  // Terzo punto
  DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
  PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);

  // Secondo punto
  ScalarEvolution &SC = AM.getResult<ScalarEvolutionAnalysis>(F);

  // Quarto punto
  DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

  // Scorrimento dei loop all'interno della Funzione

  for (auto iteraz = LI.begin(); iteraz != LI.end(); ++iteraz) {

    // Calcolo del Primo ciclo al quale siamo all'interno
    Loop *Primo = *iteraz;
    // Calcolo del secondo ciclo al quale siamo all'interno
    Loop *Secondo = *(iteraz + 1);

    if (!Secondo) {
      outs() << "Il secondo è nullo!\n";
      break;
    }

    // Effettuazione dei controlli
    bool first = Controllo_1(Primo, Secondo);
    bool secondo = Controllo_2(Primo, Secondo, SC);
    bool terzo = Controllo_3(Primo, Secondo, DT, PDT);
    bool quarto = Controllo_4(Primo, Secondo, DI);

    if (first && secondo && terzo && quarto) {
      outs() << "I due loop sono adiacenti\n";
      // In caso di requisiti soddisfatti allora è possibile ristrutturare il
      // Primo ciclo
      Primo = Ottimizza(Primo, Secondo, F, AM, LI, SC);
    } else {
      outs() << first << "\n"
             << secondo << "\n"
             << terzo << "\n"
             << quarto << "\n"
             << "Due cicli non adiacenti\n";
    }
  }

  return PreservedAnalyses::all();
}
