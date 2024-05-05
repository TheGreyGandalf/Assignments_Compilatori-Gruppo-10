#import "@preview/problemst:0.1.0": pset                      //Template per assignment
#import "@preview/tablex:0.0.8": tablex,hlinex, vlinex, rowspanx, colspanx   //Per tabelle più interessanti

#show: pset.with(
  class: "Compilatori, Gruppo 10",
  student: "Manuele Baracchi, Francesca Neri, Emanuele Ugolotti",
  title: "Assignment 2",
  date: datetime.today(),
)

#figure(image("./immagini/logo.png", width: 25%))
#outline()
#pagebreak()

= Very Busy Expressions
#box(figure(image("./immagini/diagramma1.png", width: 35%)))
#box(table(
//Tabella dataflow
columns: (auto, auto),
  inset: 10pt,
  align: horizon,
  table.header(
    [], [*Dataflow problem 1*],
  ),
  "Domain","Espressioni",
  "Direction","Backward",
  "Transfer function", $"GEN"(B) union ("IN"[B] - "KILL"[B]) $,
  "Meet operator", [$sect$, Intersezione] ,
  "Boundary condition", $"OUT"[B]_("Finale") = emptyset$,
  "Initial Interior points", $"IN"[B]_("Inizio") = emptyset$,
))

#linebreak()
//tabella GEN/KILL
#figure(
table(
columns: (auto, auto, auto),
  inset: 10pt,
  align: horizon,
  table.header(
    [], [*GEN*], [*KILL*],
  ),
  "BB1", "/", "/",
  "BB2", "/", "/",
  "BB3", "x", "BB4, BB7",
  "BB4", "x", "BB3, BB7",
  "BB5", "y", "/",
  "BB6", "a", "/",
  "BB7", "x", "BB3, BB4",
  "BB8", "/", "/",
),
caption: [Tabella GEN/KILL]
)
#figure(
///Prova di tabella con doppia intestazione
tablex(
  columns: 5,
  align: center + horizon,
  //auto-vlines: false,
  repeat-header: true,
  /* --- header --- */
  rowspanx(2)[], colspanx(2)[*Prima iterazione*], colspanx(2)[*Seconda iterazione*], 
  (),           [*IN[BB]*], [*OUT[BB]*],          [*IN[BB]*], [*OUT[BB]*],(),
  /* -------------- */
  "BB8", [$emptyset$], [$emptyset$], [$emptyset union ((a-b)- emptyset) =\ (a-b)$] , $emptyset$,  
  "BB7", [$emptyset$], [$(a-b)$], [$(a-b) union (emptyset - x)*(a-b)$   \ NC],[$(a-b)$],
  "BB6", [$emptyset$], [$(a-b)$], [$emptyset union ((a-b) - (a-b)(b-a)) =\ emptyset$   NC],[$(a-b)$],
  "BB5", [$emptyset$], [$(b-a)$], [$(b-a) union (emptyset -y)=\ (b-a)$],[$emptyset$],
  "BB4", [$emptyset$], [$(a-b)$], [$(b-a) union (emptyset - (b-a)(a-b))\ = (a)(b-a)$],[],
  "BB3", [$emptyset$], [$(b-a)$], [$b-a union (b-a-x) =\ (b-a)$], [$(b-a)$],
  "BB2", [$emptyset$], [$(a!=b)$], [$a!=b union (a!=b - emptyset) =\ a!=b$] , [$(a!=b)$],
  "BB1", [$emptyset$], [$emptyset$], [], [],
),
caption: [Tabella Iterazioni]
)

#pagebreak()
= Dominator Analysis
#box(image("./immagini/diagramma2.png", width: 35%))
#box(table(
columns: (auto, auto),
  inset: 10pt,
  align: horizon,
  table.header(
    [], [*Dataflow problem 2*],
  ),
  "Domain", "Blocchi",
  "Direction",[Forward\ OUT[b]= $f_b ("IN[b]")$\ IN[B] = $and$ OUT[pred[b]]],
  "Transfer function", [$f_b (x)={b} union x$],
  "Meet operator", [$sect$],
  "Boundary condition", [OUT[Entry] = Entry],
  "Initial Interior points", [OUT[B]=$U$],
))


#figure(
table(
columns: (auto, auto, auto),
  inset: 10pt,
  align: horizon,
  table.header(
    [], [*IN[B]*], [*OUT[B]*],
  ),
  "A", [/], [{A}],
  "B", [OUT[A]={A}], [{B} $union$ IN[B] =\ {B} $union {A} =$ {A,B}],
  "C", [OUT[A]={A}], [{C} $union$ IN[C] =\ {C} $union {A} =$ {A,C}],
  "D", [OUT[C]={A,C}], [{D} $union$ IN[D] =\ {D} $union {A,C} =$ {A,C,D}],
  "E", [OUT[C]={A,C}], [{E} $union$ IN[E] =\ {E} $union {A,C} =$ {A,C,E}],
  "F", [OUT[D] $sect$ OUT[E] =\ {A,C,D} $sect$ {A,C,E} =\ {A,C} ], [{F} $union$ IN[F]
    =\ {F} $union {A,C} =$ {A,C,F}],
  "G", [OUT[B] $sect$ OUT[F] =\ {A,B} $sect$ {A,C,F} =\ {A} ], [{G} $union$ IN[G]
    =\ {G} $union {A} =$ {A,G}],
),
caption: [Tabella Iterazioni]
)

#pagebreak()
= Constant Propagation
#box(figure(image("./immagini/diagramma3.png", width: 30%)))
#box(table(
columns: (auto, auto),
  inset: 10pt,
  align: horizon,
  table.header(
    [], [*Dataflow problem 3*],
  ),
  "Domain", [Insieme di coppie \<Variabile, Valore\>],
  "Direction", [Forward, $"OUT"_B$ = $f_b$($"IN"_B$)\ $"IN"_B$ = ($and$ OUT[pred(B)] )],
  "Transfer function", [$f_b (x)="GEN"_B union (x - "Kill"_B)$],
  "Meet operator", [$sect$, Intersezione],
  "Boundary condition", [out[entry]=$emptyset$],
  "Initial Interior points", [OUT[B]=$U$],
))

#figure(
table(
columns: (auto, auto, auto),
  inset: 10pt,
  align: horizon,
  table.header(
    [], [*GEN*], [*KILL*],
  ),
  "BB1", "k", "BB7, BB12",
  "BB2", "/", "/",
  "BB3", "a", "BB5",
  "BB4", "x", "BB6, BB10",
  "BB5", "a", "BB3",
  "BB6", "x", "BB4, BB10",
  "BB7", "k", "BB1, BB12",
  "BB8", "/", "/",
  "BB9", "b", "/",
  "BB10", "x", "BB4, BB6",
  "BB11", "y", "/",
  "BB12", "k", "BB7, BB1",
  "BB13", "/", "/",
),
caption: [Tabella GEN/KILL]
)


#figure(
tablex(
  columns: 5,
  align: center + horizon,
  //auto-vlines: false,
  repeat-header: true,
  /* --- header --- */
  rowspanx(2)[], colspanx(2)[*Prima iterazione*], colspanx(2)[*Seconda iterazione*], 
  (),           [*IN[BB]*], [*OUT[BB]*],          [*IN[BB]*], [*OUT[BB]*],(),
  /* -------------- */
  "BB1", [OUT[entry]], [${(k,2)}$], [OUT[entry]], [${(k,2)}$],
  "BB2", [OUT[BB1]], [${(k,2)}$], [OUT[BB1]], [${(k,2)}$],
  "BB3", [OUT[BB2]], [${(a,4),(k,2)}$], [OUT[BB2]], [${(a,4),(k,2)}$] ,
  "BB4", [OUT[BB3]], [${(x,5),(a,4),(k,2)}$], [OUT[BB3]], [${(x,5),(a,4),(k,2)}$],
  "BB5", [OUT[BB2]], [${(a,4),(k,2)}$], [OUT[BB2]], [${(a,4),(k,2)}$],
  "BB6", [OUT[BB5]], [${(x,8),(a,4),(k,2)}$], [OUT[BB5]], [${(x,8),(a,4),(k,2)}$],
  "BB7", [OUT[BB4] $union$ OUT[BB6]], [${(a,4),(k,4)}$],[OUT[BB4] $union$ OUT[BB6]], [${(a,4),(k,4)}$],
  "BB8", [OUT[BB7] $union$ OUT[BB12]], [${(a,4),(k,4)}$],[OUT[BB7] $union$ OUT[BB12]], [${(a,4)}$],
  "BB9", [OUT[BB8]], [${(b,2),(a,4)}$],[OUT[BB8]], [${(b,2),(a,4)}$],
  "BB10", [OUT[BB9]], [${(x,8),(b,2),(a,4)}$], [OUT[BB9]], [${(b,2),(a,4)}$],
  "BB11", [OUT[BB10]], [${(y,8),(b,2),(a,4)}$], [OUT[BB10]], [${(y,8),(b,2),(a,4)}$],
  "BB12", [OUT[BB11]], [${(k,5),(y,8),(x,8),\ (b,2),(a,4)}$], [OUT[BB11]], [${(y,8),(b,2),(a,4)}$],
  "BB13", [OUT[BB8]], [${(a,4)}$], [OUT[BB8]], [${(a,4)}$],
),
caption: [Tabella Iterazioni, convergenza ottenuta nella terza iterazione, non vi sono più cambiamenti]
)

