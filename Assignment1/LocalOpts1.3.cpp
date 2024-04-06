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
//progetto di gruppo

bool runOnBasicBlock(BasicBlock &B) {

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
                else{  //Caso in cui vi è una operazione di log "con resto"
                        llvm::APInt CPlusOne = C->getValue() + 1;
                        llvm::APInt CMinusOne = C->getValue() - 1;
                        if (CMinusOne.isPowerOf2()) {
                            Instruction *ShiftInst = BinaryOperator::Create(Instruction::Shl, Inst.getOperand(0), ConstantInt::get(C->getType(), CMinusOne.logBase2()), "", &Inst);
                            Inst.replaceAllUsesWith(ShiftInst);
                            ConstantInt *oneConstant = ConstantInt::get(C->getType(), 1);
                            Instruction *SubInst = BinaryOperator::Create(Instruction::Sub, Inst.getOperand(0), oneConstant, "", &Inst);
                            SubInst->insertAfter(&Inst);
                            continue;
                        }
                        if (CPlusOne.isPowerOf2()) {
                            Instruction *ShiftInst = BinaryOperator::Create(Instruction::Shl, Inst.getOperand(0), ConstantInt::get(C->getType(), CPlusOne.logBase2()), "", &Inst);
                            Inst.replaceAllUsesWith(ShiftInst);
                            ConstantInt *oneConstant = ConstantInt::get(C->getType(), 1);
                            Instruction *AddInst = BinaryOperator::Create(Instruction::Add, Inst.getOperand(0), oneConstant, "", &Inst);
                            AddInst->insertAfter(&Inst);
                            continue;
                        }                        
                    }
            }
        }
        else if (Inst.getOpcode() == Instruction::SDiv) {
        //Caso in cui si cerca di ottimizzare una DIVISIONE 
        //Le operazioni seguenti sono pressochè identiche a quelle per la moltiplicazione
        // Verifica se il secondo operando è una costante
            if (auto *C = dyn_cast<ConstantInt>(Inst.getOperand(1))) {
                // Verifica se la costante è una potenza di 2
                if (C->getValue().isPowerOf2()) {
                    // Crea un'istruzione di shift
                    Instruction *ShiftInst = BinaryOperator::Create(
                    Instruction::AShr, Inst.getOperand(0), //Ashr shift a destra
                    ConstantInt::get(C->getType(), C->getValue().logBase2()), "",
                    &Inst);

                    // Sostituisci tutte le occorrenze dell'istruzione di moltiplicazione
                    // con l'istruzione di shift
                    Inst.replaceAllUsesWith(ShiftInst);
                    continue;
                }
            }
        }
            

        // Ottimizzazione per x + 0 o 0 + x
        if (Inst.getOpcode() == Instruction::Add) {
            //prendo i due operandi 
            Value *Op0 = Inst.getOperand(0);
            Value *Op1 = Inst.getOperand(1);
            if (auto *Op0Const = dyn_cast<ConstantInt>(Op0)) {
                if (Op0Const->isZero()) {
                    Inst.replaceAllUsesWith(Op1);
                    //Inst.eraseFromParent();             commentanto perhè mi dava errore di segmentation fault
                    continue;
                }
            } else if (auto *Op1Const = dyn_cast<ConstantInt>(Op1)) {
                if (Op1Const->isZero()) {
                    Inst.replaceAllUsesWith(Op0);
                    //Inst.eraseFromParent();             commentanto perhè mi dava errore di segmentation fault
                    continue;
                }
            }
        }

        // Controllo se l'istruzione è una ADD
        // Controllo se l'istruzione è una ADD
if (Inst.getOpcode() == Instruction::Add) {
    // Verifico se l'istruzione di addizione ha un solo utilizzo
    if (Inst.getNumUses() == 1) {
        Instruction *NextInst = dyn_cast<Instruction>(*Inst.user_begin());
        // Controllo se la prossima istruzione è una SUB e non nulla
        if (NextInst && NextInst->getOpcode() == Instruction::Sub) {
            // Ottengo gli operandi delle due operazioni
            Value *AddOperand = Inst.getOperand(1); // b è il secondo operando
            Value *SubOperand = NextInst->getOperand(1); // b è il secondo operando
            // Controllo se l'operando di addizione e sottrazione è lo stesso
            if (AddOperand == SubOperand) {
                // Creo un'istruzione di addizione a 0
                Instruction *InstAddOp = BinaryOperator::Create(
                    Instruction::Add, Inst.getOperand(0),
                    ConstantInt::get(Inst.getType(), 0));

                // Inserisco l'istruzione di addizione dopo l'istruzione corrente
                InstAddOp->insertAfter(&Inst);
                
                // Sostituisco tutti gli utilizzi dell'istruzione corrente con l'istruzione di addizione
                Inst.replaceAllUsesWith(InstAddOp);
                
                // Rimuovo l'istruzione corrente
                Inst.eraseFromParent();
                
                // Rimuovo anche l'istruzione di sottrazione successiva
                NextInst->eraseFromParent();
                
                // Continuo con la prossima iterazione del ciclo
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

