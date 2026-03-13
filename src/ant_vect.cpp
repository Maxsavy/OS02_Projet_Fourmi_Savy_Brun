#include "ant_vect.hpp"

void advance_Vect(VectorOfAnts& MyVector, pheronome& phen, const fractal_land& land, 
                  const position_t& pos_food, const position_t& pos_nest, size_t& cpteur_food) 
{
    const size_t nb_ants = MyVector.pos_x.size();
    const double eps =MyVector.m_eps; // On utilise la constante locale pour faciliter la vectorisation
    
    // On utilise OpenMP pour paralléliser la boucle de calcul des fourmis
    // reduction(+:cpteur_food) permet de cumuler les résultats de chaque thread en toute sécurité
    #pragma omp parallel for reduction(+:cpteur_food) schedule(dynamic, 64)
    for (std::size_t i = 0; i < nb_ants; ++i) {
        
        double consumed_time = 0.0;
        
        // La fourmi continue tant qu'elle a des points de mouvement
        while (consumed_time < 1.0) {
            int ind_pher = (MyVector.states[i] == 1 ? 1 : 0); // État chargée ou non
            
            position_t old_pos = {MyVector.pos_x[i], MyVector.pos_y[i]};
            position_t new_pos = old_pos;

            // Calcul du maximum des phéromones environnants
            double p_up    = phen(old_pos.x,     old_pos.y - 1)[ind_pher];
            double p_down  = phen(old_pos.x,     old_pos.y + 1)[ind_pher];
            double p_left  = phen(old_pos.x - 1, old_pos.y    )[ind_pher];
            double p_right = phen(old_pos.x + 1, old_pos.y    )[ind_pher];

            double max_phen = std::max({p_up, p_down, p_left, p_right});

            // Choix de la direction : Exploration (aléatoire) ou Exploitation (phéromones)
            double choix = rand_double(0., 1., MyVector.seeds[i]);

            if ((choix > eps) || (max_phen <= 0.0)) {
                // Exploration ou aucune piste : choix aléatoire d'une cellule non bloquante
                int d;
                do {
                    new_pos = old_pos;
                    d = rand_int32(1, 4, MyVector.seeds[i]);
                    if      (d == 1) new_pos.x -= 1;
                    else if (d == 2) new_pos.y -= 1;
                    else if (d == 3) new_pos.x += 1;
                    else             new_pos.y += 1;
                } while (phen[new_pos][ind_pher] == -1); // Évite les cellules indésirables
            } 
            else {
                // Exploitation : on suit la piste la plus forte
                if      (p_left  == max_phen) new_pos.x -= 1;
                else if (p_right == max_phen) new_pos.x += 1;
                else if (p_up    == max_phen) new_pos.y -= 1;
                else                          new_pos.y += 1;
            }

            // Mise à jour de la fourmi et de l'environnement
            consumed_time += land(new_pos.x, new_pos.y); // Coût de déplacement selon le terrain
            MyVector.pos_x[i] = new_pos.x;
            MyVector.pos_y[i] = new_pos.y;

            // Vérification des objectifs (Nid ou Nourriture)
            if (new_pos.x == pos_nest.x && new_pos.y == pos_nest.y) {
                if (MyVector.states[i] == 1) cpteur_food += 1; //
                MyVector.states[i] = 0; // Devient "non chargée"
            }
            if (new_pos.x == pos_food.x && new_pos.y == pos_food.y) {
                MyVector.states[i] = 1; // Devient "chargée"
            }
        }
    }
    //On sépare la pose des phéromones pour éviter un problème d'accès à la mémoire partagée et de verrous intempestifs
    for (std::size_t i = 0; i < nb_ants; ++i) {
        phen.mark_pheronome({MyVector.pos_x[i], MyVector.pos_y[i]}); // La fourmi dépose ses phéromones
    }
}