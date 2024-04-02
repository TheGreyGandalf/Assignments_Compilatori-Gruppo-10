//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
// L'include seguente va in LocalOpts.h
#include <llvm/IR/Constants.h>

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
	       <<" DELLA FUNZIONE " << Arg->getParent()->getName()
               << "\n";
      }
      if (ConstantInt *C = dyn_cast<ConstantInt>(Operand)) {
        outs() << "\t" << *C << ": SONO UNA COSTANTE INTERA DI VALORE " << C->getValue()
               << "\n";
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
    Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(1));

    NewInst->insertAfter(&Inst1st);
    // Si possono aggiornare le singole references separatamente?
    // Controlla la documentazione e prova a rispondere.
    Inst1st.replaceAllUsesWith(NewInst);


  // All'interno della funzione runOnBasicBlock
  //FATTO DA ME
  // Itera attraverso tutte le istruzioni del blocco di base
  for (auto &Inst : B) {
      // Verifica se l'istruzione è di tipo moltiplicazione
      if (Inst.getOpcode() == Instruction::Mul) {
          // Verifica se il secondo operando è una costante
          if (auto *C = dyn_cast<ConstantInt>(Inst.getOperand(1))) {
              // Verifica se la costante è una potenza di 2
              if (C->getValue().isPowerOf2()) {
                  // Crea un'istruzione di shift
                  Instruction *ShiftInst = BinaryOperator::Create( Instruction::Shl, Inst.getOperand(0), ConstantInt::get(C->getType(), C->getValue().logBase2()), "", &Inst);
                  
                  // Sostituisci tutte le occorrenze dell'istruzione di moltiplicazione con l'istruzione di shift
                  Inst.replaceAllUsesWith(ShiftInst);
                  continue;
              }
          }
      }

      // Ottimizzazione per x + 0 o 0 + x
      if (Inst.getOpcode() == Instruction::Add) {
          Value *Op0 = Inst.getOperand(0);
          Value *Op1 = Inst.getOperand(1);
          Constant *Zero = ConstantInt::get(Op0->getType(), 0);
          if (Op0 == Zero) {
              Inst.replaceAllUsesWith(Op1);
              Inst.eraseFromParent();
              continue;
          } else if (Op1 == Zero) {
              Inst.replaceAllUsesWith(Op0);
              Inst.eraseFromParent();
              continue;
          }
      }

      // Ottimizzazione per la moltiplicazione per 1
      if (Inst.getOpcode() == Instruction::Mul) {
          Value *Op0 = Inst.getOperand(0);
          Value *Op1 = Inst.getOperand(1);
          Constant *One = ConstantInt::get(Op0->getType(), 1); 
          if (Op0 == One || Op1 == One) {
              Inst.replaceAllUsesWith(Op0 == One ? Op1 : Op0);
              Inst.eraseFromParent();
              continue;
          }
      }
      return true;
  }
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

