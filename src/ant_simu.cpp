#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "fractal_land.hpp"
#include "ant.hpp"
#include "pheronome.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include "rand_generator.hpp"

using namespace std::chrono;

auto start = steady_clock::now();
auto end   = steady_clock::now();

void advance_time(const fractal_land &land, pheronome &phen,
                  const position_t &pos_nest, const position_t &pos_food,
                  std::vector<ant> &ants, std::size_t &cpteur)
{
    for (size_t i = 0; i < ants.size(); ++i)
        ants[i].advance(phen, land, pos_food, pos_nest, cpteur);
    phen.do_evaporation();
    phen.update();
}

int main(int nargs, char *argv[])
{
    //DEFINITION DE CONSTANTES

    SDL_Init(SDL_INIT_VIDEO);
    std::size_t seed = 2026;  // Graine pour la génération aléatoire ( reproductible )
    const int nb_ants = 5000; // Nombre de fourmis
    const double eps = 0.8;   // Coefficient d'exploration
    const double alpha = 0.7; // Coefficient de chaos
    // const double beta=0.9999; // Coefficient d'évaporation
    const double beta = 0.999; // Coefficient d'évaporation
    // Location du nid
    position_t pos_nest{256, 256};
    // Location de la nourriture
    position_t pos_food{500, 500};
    // const int i_food = 500, j_food = 500;

    //TERRITOIRE :

    //  Génération du TERRITOIRE 512 x 512 ( 2*(2^8) par direction )
    auto generation_Time_start = steady_clock::now();
    fractal_land land(8, 2, 1., 1024);
    double max_val = 0.0;
    double min_val = 0.0;
    for (fractal_land::dim_t i = 0; i < land.dimensions(); ++i)
        for (fractal_land::dim_t j = 0; j < land.dimensions(); ++j)
        {
            max_val = std::max(max_val, land(i, j));
            min_val = std::min(min_val, land(i, j));
        }
    double delta = max_val - min_val;
    /* On redimensionne les valeurs de fractal_land de sorte que les valeurs
    soient comprises entre zéro et un */
    for (fractal_land::dim_t i = 0; i < land.dimensions(); ++i)
        for (fractal_land::dim_t j = 0; j < land.dimensions(); ++j)
        {
            land(i, j) = (land(i, j) - min_val) / delta;
        }
    auto generation_Time_end = steady_clock::now();
    auto generation_Duration = generation_Time_end-generation_Time_start;
    std::cout << "Durée de génération du terrain : " << duration_cast<milliseconds>(generation_Duration).count() << " ms" << std::endl;

    //FOURMIS :

    auto Fourmis_generation_Time_start = steady_clock::now();
    // Définition du coefficient d'exploration de toutes les FOURMIS.
    ant::set_exploration_coef(eps);
    // On va créer des fourmis un peu partout sur la carte :
    std::vector<ant> ants;
    ants.reserve(nb_ants);
    auto gen_ant_pos = [&land, &seed]()
    { return rand_int32(0, land.dimensions() - 1, seed); };
    for (size_t i = 0; i < nb_ants; ++i)
        ants.emplace_back(position_t{gen_ant_pos(), gen_ant_pos()}, seed);
    // On crée toutes les fourmis dans la fourmilière.
    auto Fourmis_generation_Time_end = steady_clock::now();
    auto Fourmis_generation_Duration = Fourmis_generation_Time_end-Fourmis_generation_Time_start;
    std::cout << "Durée de génération initiale des fourmis : " << duration_cast<milliseconds>(Fourmis_generation_Duration).count() << " ms" << std::endl;

    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    Window win("Ant Simulation", 2 * land.dimensions() + 10, land.dimensions() + 266);
    Renderer renderer(land, phen, pos_nest, pos_food, ants);

    // Compteur de la quantité de nourriture apportée au nid par les fourmis
    size_t food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    
    //on définit ici des constantes pour calculer une moyenne de la durée de calcul d'une génération
    double total_duration = 0;
    int total_count=0;

    while (cont_loop)
    {
        ++it;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                cont_loop = false;
        }

        auto New_generation_Time_start = steady_clock::now();
        advance_time(land, phen, pos_nest, pos_food, ants, food_quantity);
        auto New_generation_Time_end = steady_clock::now();

        total_duration+=duration_cast<microseconds>(New_generation_Time_end-New_generation_Time_start).count();
        total_count++;

        renderer.display(win, food_quantity);
        win.blit();

        if (total_count % 100 == 0) {
        std::cout << "Moyenne durée du calcul nouvelle génération sur (100 frames): " << (total_duration / 100.0) / 1000.0 << " ms" << std::endl;
        total_duration = 0;
        }

        if (not_food_in_nest && food_quantity > 0)
        {
            std::cout << "La première nourriture est arrivée au nid a l'iteration " << it << std::endl;
            not_food_in_nest = false;
        }
        // SDL_Delay(10);
    }
    SDL_Quit();
    return 0;
}