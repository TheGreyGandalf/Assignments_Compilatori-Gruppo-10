//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
// L'include seguente va in LocalOpts.h
#include <cmath>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/Alignment.h>

using namespace llvm;

bool runOnBasicBlock(BasicBlock &B) {

  // Preleviamo le prime due istruzioni del BB
  Instruction &Inst1st = *B.begin(), &Inst2nd = *(++B.begin());

  // L'indirizzo della prima istruzione deve essere uguale a quello del
  // primo operando della seconda istruzione (per costruzione dell'esempio)
  assert(&Inst1st == Inst2nd.getOperand(0));

  // Stampa la prima istruzione
  outs() << "PRIMA ISTRUZIONE: " << Inst1st << "\n";
  // Stampa la prima istruzione come operando
  outs() << "COME OPERANDO: ";
  Inst1st.printAsOperand(outs(), false);
  outs() << "\n";

  // User-->Use-->Value
  outs() << "I MIEI OPERANDI SONO:\n";
  for (auto *Iter = Inst1st.op_begin(); Iter != Inst1st.op_end(); ++Iter) {
    Value *Operand = *Iter;

    if (Argument *Arg = dyn_cast<Argument>(Operand)) {
      outs() << "\t" << *Arg << ": SONO L'ARGOMENTO N. " << Arg->getArgNo()
             << " DELLA FUNZIONE " << Arg->getParent()->getName() << "\n";
    }
    if (ConstantInt *C = dyn_cast<ConstantInt>(Operand)) {
      outs() << "\t" << *C << ": SONO UNA COSTANTE INTERA DI VALORE "
             << C->getValue() << "\n";
    }
  }

  outs() << "LA LISTA DEI MIEI USERS:\n";
  for (auto Iter = Inst1st.user_begin(); Iter != Inst1st.user_end(); ++Iter) {
    outs() << "\t" << *(dyn_cast<Instruction>(*Iter)) << "\n";
  }

  outs() << "E DEI MIEI USI (CHE E' LA STESSA):\n";
  for (auto Iter = Inst1st.use_begin(); Iter != Inst1st.use_end(); ++Iter) {
    outs() << "\t" << *(dyn_cast<Instruction>(Iter->getUser())) << "\n";
  }

  // Manipolazione delle istruzioni
  Instruction *NewInst = BinaryOperator::Create(
      Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(0));

  NewInst->insertAfter(&Inst1st);
  // Si possono aggiornare le singole references separatamente?
  // Controlla la documentazione e prova a rispondere.
  Inst1st.replaceAllUsesWith(NewInst);

  // Codice che aggiunge una istruzione di Shift

  for (auto &Inst : B) {
    // Verifica se l'istruzione è di tipo moltiplicazione
    if (Inst.getOpcode() == Instruction::Mul) {

      // Ottimizzazione della moltiplicazione per 1
      Value *Op0 = Inst.getOperand(0);
      Value *Op1 = Inst.getOperand(1);
      if (auto *Op0Const = dyn_cast<ConstantInt>(Op0)) {
        if (Op0Const->equalsInt(1)) {
          Inst.replaceAllUsesWith(Op1);
          // Inst.eraseFromParent();     commentanto perhè mi dava errore di
          // segmentation fault
          continue;
        }
      } else if (auto *Op1Const = dyn_cast<ConstantInt>(Op1)) {
        if (Op1Const->equalsInt(1)) {
          Inst.replaceAllUsesWith(Op0);
          // Inst.eraseFromParent();     commentanto perhè mi dava errore di
          // segmentation fault
          continue;
        }
      }

      // Caso in cui si cerca di ottimizzare una MOLTIPLICAZIONE
      //  Verifica se il secondo operando è una costante
      if (auto *C = dyn_cast<ConstantInt>(Inst.getOperand(1))) {
        // Verifica se la costante è una potenza di 2
        if (C->getValue().isPowerOf2()) {
          // Crea un'istruzione di shift
          Instruction *ShiftInst = BinaryOperator::Create(
              Instruction::Shl, Inst.getOperand(0),
              ConstantInt::get(C->getType(), C->getValue().logBase2()), "",
              &Inst);

          // Sostituisci tutte le occorrenze dell'istruzione di moltiplicazione
          // con l'istruzione di shift
          Inst.replaceAllUsesWith(ShiftInst);
          continue;
        } else { // Caso in cui vi è una operazione di log "con resto"
                 //  Il resto è ottenibile mediante C->getValue().logBase2()
                 // Caso in cui vi è una operazione di log "con resto"
          llvm::APInt CPlusOne = C->getValue() + 1;
          llvm::APInt CMinusOne = C->getValue() - 1;
          if (CMinusOne.isPowerOf2()) {
            Instruction *ShiftInst = BinaryOperator::Create(
                Instruction::Shl, Inst.getOperand(0),
                ConstantInt::get(C->getType(), CMinusOne.logBase2()), "",
                &Inst);
            Inst.replaceAllUsesWith(ShiftInst);
            ConstantInt *oneConstant = ConstantInt::get(C->getType(), 1);
            Instruction *SubInst = BinaryOperator::Create(
                Instruction::Sub, Inst.getOperand(0), oneConstant, "", &Inst);
            SubInst->insertAfter(&Inst);
            continue;
          }
          if (CPlusOne.isPowerOf2()) {
            Instruction *ShiftInst = BinaryOperator::Create(
                Instruction::Shl, Inst.getOperand(0),
                ConstantInt::get(C->getType(), CPlusOne.logBase2()), "", &Inst);
            Inst.replaceAllUsesWith(ShiftInst);
            ConstantInt *oneConstant = ConstantInt::get(C->getType(), 1);
            Instruction *AddInst = BinaryOperator::Create(
                Instruction::Add, Inst.getOperand(0), oneConstant, "", &Inst);
            AddInst->insertAfter(&Inst);
            continue;
          }
        }
      }
    }
    if (Inst.getOpcode() == Instruction::SDiv) { // La U sta per Unsigned
      // Caso in cui si cerca di ottimizzare una DIVISIONE
      // Le operazioni seguenti sono pressochè identiche a quelle per la
      // moltiplicazione
      // Verifica se il secondo operando è una costante
      if (auto *C = dyn_cast<ConstantInt>(Inst.getOperand(1))) {
        // Verifica se la costante è una potenza di 2
        if (C->getValue().isPowerOf2()) {
          // Crea un'istruzione di shift
          Instruction *ShiftInst = BinaryOperator::Create(
              Instruction::AShr, Inst.getOperand(0),
              ConstantInt::get(C->getType(), C->getValue().logBase2()), "",
              &Inst);

          // Sostituisci tutte le occorrenze dell'istruzione di moltiplicazione
          // con l'istruzione di shift
          Inst.replaceAllUsesWith(ShiftInst);
          continue;
        }
      }
    }
    if (Inst.getOpcode() == Instruction::Add) {
      // Ultimo punto dell'assignment

      // Controllo se è una ADD che utilizza un valore una volta sola e se è
      // seguita da una SUB, quindi ci troviamo nel caso di una ottimizzazione
      // che è possibile eseguire!
      if (Inst.getNumUses() == 1) {
        Instruction *NextInst = dyn_cast<Instruction>(*Inst.user_begin());
        // Cerco se la prossima istruzione è una SUB e non nulla
        if (NextInst && NextInst->getOpcode() == Instruction::Sub) {
          // Salvataggio del valore che l'istruzione andava a generare
          Value *AddResult = &Inst;
          // Salvo quali sono gli utilizzatori della sub che poi si eliminerà
          Instruction *SubInst = cast<Instruction>(Inst.use_begin()->getUser());

          // Ottengo gli operandi delle due operazioni
          Value *AddOperand = Inst.getOperand(1); // b è il secondo operando
          Value *SubOperand = SubInst->getOperand(1); // b è il secondo operando

          // Controllo se l'operando di addizione e sottrazione è lo stesso
          if (AddOperand == SubOperand) {

            // Il mio intento è aggiungere una istruzione che faccia la stessa
            // cosa Invece di eliminare le istruzioni, aggiungo in fondo
            // l'istruzione che mi consente di riassumere il compito 

            Instruction *InstAddOp = BinaryOperator::Create(
                Instruction::Add, Inst.getOperand(1),
                ConstantInt::getNullValue(Inst.getType()));

            // Inserisco l'istruzione di addizione dopo l'istruzione corrente

            InstAddOp->insertAfter(&Inst);
            // Sostituisco tutti gli utilizzi dell'istruzione corrente con
            // l'istruzione di addizione
            Inst.replaceAllUsesWith(InstAddOp);
            // SubInst->replaceAllUsesWith(InstAdd);

            // Rimuovo l'istruzione corrente
            //Inst.eraseFromParent();

            // Alla fine rimpiazzo la sub con una add a 0
            //  Si rimpiazzano tutti gli utilizzi della operazione Inst con b

            continue;
          }
        }
      } else { // Parte scritta dal mio compagno di gruppo (In cui i casi di
               // utilizzo non sono solamente uno)

        // prendo i due operandi
        Value *Op0 = Inst.getOperand(0);
        Value *Op1 = Inst.getOperand(1);
        if (auto *Op0Const = dyn_cast<ConstantInt>(Op0)) {
          if (Op0Const->isZero()) {
            Inst.replaceAllUsesWith(Op1);
            // Inst.eraseFromParent();             commentanto perhè mi dava
            // errore di segmentation fault
            continue;
          }
        } else if (auto *Op1Const = dyn_cast<ConstantInt>(Op1)) {
          if (Op1Const->isZero()) {
            Inst.replaceAllUsesWith(Op0);
            // Inst.eraseFromParent();             commentanto perhè mi dava
            // errore di segmentation fault
            continue;
          }
        }
      }
    }
  }

  return true;
}

bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}

PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();

  return PreservedAnalyses::all();
}
