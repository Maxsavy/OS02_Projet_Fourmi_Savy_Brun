#pragma once
#include <omp.h>
#include <iostream>
# include <utility>
#include <random>
# include "pheronome.hpp"
# include "fractal_land.hpp"
# include "basic_types.hpp"
#include "rand_generator.hpp"

class VectorOfAnts {    //Chaque fourmie peut être déterminée à partir d'un indice dans cette classe
    public :
        VectorOfAnts(double eps, int n) : m_eps(eps), numbAnts(n) {
            pos_x.resize(n);
            pos_y.resize(n);
            states.assign(n, 0);        //toutes les fourmis commencent non chargées
            seeds.resize(n);
            consumed_time.assign(n, 0); 
            std::mt19937 master_gen(2026); // Générateur maître
            for (int i = 0; i < numbAnts; ++i) {
                // Chaque fourmi reçoit une graine de départ différente
                seeds[i] = master_gen(); 
            }
        }
        std::vector<int> pos_x;  //vecteur des positions des fourmies
        std::vector<int> pos_y;
        std::vector<int> states;  //vecteur des états des fourmies (chargée ou non) (1 si oui, 0 sinon)
        std::vector<size_t> seeds; //la graine qui détermine le hasard pour chaque fourmie
        std::vector<double> consumed_time;
        double m_eps;      // Coefficient d'exploration commun à toutes les fourmis.
        int numbAnts;

        void gen_Ants_pos(const fractal_land &land) {
            
            std::transform(seeds.begin(), seeds.end(), pos_x.begin(), [&land](size_t& seed) {
                return rand_int32(0, land.dimensions()-1, seed);
            });
            std::transform(seeds.begin(), seeds.end(), pos_y.begin(), [&land](size_t& seed) {
                return rand_int32(0, land.dimensions()-1, seed);
            });
        }
};


void advance_Vect(VectorOfAnts& MyVector, pheronome& phen, const fractal_land& land, 
                  const position_t& pos_food, const position_t& pos_nest, size_t& cpteur_food);