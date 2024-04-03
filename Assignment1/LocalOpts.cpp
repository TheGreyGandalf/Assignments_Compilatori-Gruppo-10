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


  //Codice che aggiunge una istruzione di Shift 

  for (auto &Inst : B) {
    // Verifica se l'istruzione è di tipo moltiplicazione
    if (Inst.getOpcode() == Instruction::Mul) {
      //Caso in cui si cerca di ottimizzare una MOLTIPLICAZIONE
      // Verifica se il secondo operando è una costante
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
          }
        else{        //Caso in cui vi è una operazione di log "con resto"
                      // Il resto è ottenibile mediante C->getValue().logBase2()
            // Operazioni ancora in fase di lavorazione e incerte, attesa di risposta da marongiu

            //Questo stampa il resto che c'è dal logaritmo
            int resto = C->getValue().logBase2();
            int num = 1; 
            for (int i = 0; i<= resto-1; i++) {
               num *= 2;
            }
            //Stampa di qual'è il valore di logaritmo che si avvicina di più
            outs() << "\t"<< "Il numero di logaritmo più basso è: "<<num<<"\n";
            outs() << "\t"<< "Il numero di logaritmo più alto è: "<<num+1<<"\n";
            int num_intero = C->getValue().getSExtValue();
            //Stampa di qual'era il valore che era stato richiesto per la shift
            outs() << "\t"<< "Il numero di moltiplicazione: "<<num_intero<<"\n";

            int appross = (num_intero<<(num+1));
            outs() << "Numero allungato in alto:"<< appross << "\n";

        }
      }
    }
    else if (Inst.getOpcode() == Instruction::UDiv) {       //La U sta per Unsigned
      //Caso in cui si cerca di ottimizzare una DIVISIONE 
       //Le operazioni seguenti sono pressochè identiche a quelle per la moltiplicazione
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
          break;
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
