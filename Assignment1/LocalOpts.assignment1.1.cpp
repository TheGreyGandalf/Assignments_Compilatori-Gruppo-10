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

          if (auto *Op0Const = dyn_cast<ConstantInt>(Op0)) {
              if (Op0Const->isZero()) {
                  Inst.replaceAllUsesWith(Op1);
                  Inst.eraseFromParent();
                  continue;
              }
          } else if (auto *Op1Const = dyn_cast<ConstantInt>(Op1)) {
              if (Op1Const->isZero()) {
                  Inst.replaceAllUsesWith(Op0);
                  Inst.eraseFromParent();
                  continue;
              }
          }
      }

      // Ottimizzazione per la moltiplicazione per 1
if (Inst.getOpcode() == Instruction::Mul) {
    Value *Op0 = Inst.getOperand(0);
    Value *Op1 = Inst.getOperand(1);
    Constant *One = ConstantInt::get(Op0->getType(), 1); 

    if (auto *Op0Const = dyn_cast<ConstantInt>(Op0)) {
        if (Op0Const->equalsInt(1)) {
            Inst.replaceAllUsesWith(Op1);
            Inst.eraseFromParent();
            continue;
        }
    } else if (auto *Op1Const = dyn_cast<ConstantInt>(Op1)) {
        if (Op1Const->equalsInt(1)) {
            Inst.replaceAllUsesWith(Op0);
            Inst.eraseFromParent();
            continue;
        }
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

