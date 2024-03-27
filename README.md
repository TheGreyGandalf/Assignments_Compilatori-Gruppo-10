# Assignments_Compilatori
Serie di assignment che saranno consegnati per Compilatori

# Testo primo Assignment:
Implementare tre passi LLVM (dentro lo stesso passo LocalOpts già scritto durante il LAB 2) che realizzano le seguenti ottimizzazioni locali:
1. Algebraic Identity
  𝑥 + 0 = 0 + 𝑥 ⇒𝑥
  𝑥 × 1 = 1 × 𝑥 ⇒𝑥
2. Strength Reduction (più avanzato)
  15 × 𝑥 = 𝑥 × 15 ⇒ (𝑥 ≪ 4) – x
  y = x / 8 ⇒ y = x >> 3
3. Multi-Instruction Optimization
  𝑎 = 𝑏 + 1, 𝑐 = 𝑎 − 1 ⇒𝑎 = 𝑏 + 1, 𝑐 = 𝑏
