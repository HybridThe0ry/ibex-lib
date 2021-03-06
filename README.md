[![Build Status](https://travis-ci.org/ibex-team/ibex-lib.svg?branch=master)](https://travis-ci.org/ibex-team/ibex-lib)
[![Build status](https://ci.appveyor.com/api/projects/status/9w1wxhvymsohs4gr/branch/master?svg=true)](https://ci.appveyor.com/project/Jordan08/ibex-lib-q0c47/branch/master)

ibex-lib
========

http://www.ibex-lib.org

Instalation
-----------

./waf configure --with-optim --with-optim-mop  --with-ampl --with-affine --prefix=. --gaol-dir= --lp-lib=soplex

./waf install

Luego resolver un problema de ejemplo:
__build__/plugins/optim-mop/ibexmop ../benchs/MOP/binh.txt --cy-contract --eps 1 -b largestfirst --nb_ub_sols 10 --plot --w2 0.01


TODO
----


Técnicas de selección de nodo:
  - [x] [OC](http://ben-martin.fr/files/publications/2016/EJOR_2016.pdf): min (z1.lb-z1_init.lb)/wid(z1_init) +  (z2.lb-z2_init.lb)/wid(z2_init)
  - [x] SR1, [OC1](https://tel.archives-ouvertes.fr/tel-01146856/document): Min lb1
  - [x] [OC2](https://tel.archives-ouvertes.fr/tel-01146856/document): Min lb2
  - [x] [OC3](https://tel.archives-ouvertes.fr/tel-01146856/document): Min lb1 + lb2
  - [x] [OC4](https://tel.archives-ouvertes.fr/tel-01146856/document): Decreasing value of
  hypervolume of the point y (considering the initial values for z1_ub and z2_ub)
  - [ ] [OC5](https://tel.archives-ouvertes.fr/tel-01146856/document): Decreasing box size
  of boxes such that lb is not dominated **(Damir's CellNSSet)**
  - [ ] Escoger caja random del Nondominated Set **(Damir)**
  - [x] Escoger caja que maximiza la distancia a UB (Optimizada usando Pqueue)
  - [x] Diving compatible con los metodos anteriores
  - [ ] Que hacer cuando aun no hay upperbounds?


**(Ignacio)** Upperbounding:
  - [x] Criterio dinamico para establecer cantidad de puntos que se generan
  - [x] Adaptar para trabajar con ecuaciones (Hansen-Sengupta)
  - [ ] Bug: Problema con PdcHansenFeasibility (LoupFinderMOP) para problema test.bch
  - [ ] Encontrar recta factible en x usando simplex,
  para luego obtener segmentos upperbound de la curva asociada en y
  - [ ] Implementar metodos para manejar el set de segmentos no dominados (nuevo UB)

Definicion del lowerbound (y eventualmente UB):
  - [x] Algoritmo para definir segmentos LB o UB **reparar bugs**



[Algoritmo para encontrar interseccion entre 2 segmentos](https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect)

Ver Vincent et al. (2013) Multiple objective branch and bound for mixed 0-1 linear programming:
Corrections and improvements for the biobjective case

Preparar experimentos:
  - [x] Agregar todos los benchmarks de este [paper](http://ben-martin.fr/files/publications/2016/EJOR_2016.pdf)
  - [x] Agregar opcion de setear cantidad de soluciones del upperbounding (nb_ub_sols)
  - [x] Agregar opcion para mostrar/no mostrar plot (plot?)
  - [x] Agregar opcion para modificar distancia minima aceptada entre soluciones factibles (min_ub_dist)

**Estructura de papers.**

*Paper 1. Nonlinear biobjective optimization. Improvements to interval Branch&Bounds algorithms*  (contribuciones):
  - Propiedad de las soluciones (ub):
  any feasible solution x is eps-dominated by at least one solution x' in the ub set, i.e.,
  for all x feasible, there exists at least one x' in ub_set, such that: f1(x') <= f1(x) + eps  and f2(x') <= f2(x) + eps
  - Crierios de seleccion del siguiente nodo (max_distance + diving?)
    - Explicar algoritmo ub_distance en detalle (usando pqueue y recalculo de distancias)
  - Upperbounding usando simplex (min f1 + min f2 + midpoint)
  - Discarding boxes by using an auxiliary constraint: z1+a*z2=w


**Experiments**

Ejemplo:

      ./optimizer-mop ../benchs/MOP/binh.txt --cy-contract-full --eps_rel=0.01 -b lsmear --nb_ub_sols 50 --w2 0.01  | sed  '$!d'

  - Para cada estrategia reportar: tiempo, #nodos, |Y|, #nb_sols
  - En las tablas las instancias deberían corresponder a las filas y las estrategias a multi-columnas de 4 columnas
  - Almacenar resultados en spreadsheets indicar commit (versión del solver utilizada)
  - Parámetros fijos: --w2 0.01 (usar en todos los experimentos)
  - Estrategia basica (std), similar a la del paper: -f hc4 -s weighted_sum --nb_ub_sols=1 -b largestfirst --no-bisecty
  - Estrategia full contractor (fullctc), usando componentes de ibexOpt: -f acidhc4 --linear-relax=compo -s weighted_sum --nb_ub_sols=1 -b largestfirst --no-bisecty
  - Estrategia full contractor + lsmear: -f acidhc4 --linear-relax=compo -s weighted_sum --nb_ub_sols=1 -b lsmear
  - [ ] upperbounding simplex + full contractor:
  	- $std --nb_ub_sols=X, X in {1, 3, 5, 10, 50, 100}
  	- $fullctc --nb_ub_sols=X, X in {1, 3, 5, 10, 50, 100}
  - [ ] Comparar estrategias de biseccion (lsmear - largestfirst-nobisect-y)
    - $fullctc --nb_ub_sols=best_X
    - $fullctc-lsmear --nb_ub_sols=best_X
  - [ ] Metodo de caja box + cy (lo que mejora la precision w_lowerbound)
    - $fullctc --nb_ub_sols=best_X (best_X es el mejor valor obtenido en la experimentación anterior)
    - $fullctc --nb_ub_sols=best_X --cy-contract
  - [ ] Metodo de caja box + cy (lo que mejora el filtrado w_upperbound)
    - $fullctc --nb_ub_sols=best_X --cy-contract-full
  - [ ] epsilon_contract
    - $fullctc --nb_ub_sols=best_X --cy-contract-full --eps-contract
  - [ ] Comparar estrategias de seleccion de nodo (NDSdist - diving-NDSdist)
    - $fullctc --nb_ub_sols=best_X --cy-contract-full --eps-contract -s weighted_sum
    - $fullctc --nb_ub_sols=best_X --cy-contract-full --eps-contract -s NDSdist
    - $fullctc --nb_ub_sols=best_X --cy-contract-full --eps-contract -s diving-NDSdist


*Paper 2. Nonlinear biobjective optimization. Improving the precision of the nondominated set by using edges.* (contribuciones):
  - Definicion del ub_set usando segmentos factibles
  - Proponer algoritmo eficiente (busqueda binaria) para encontrar soluciones factibles asociadas a puntos en los segmentos
