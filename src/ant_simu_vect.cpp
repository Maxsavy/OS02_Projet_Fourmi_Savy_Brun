#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include "fractal_land.hpp"
#include "ant_vect.hpp"
#include "pheronome.hpp"
#include "renderer_vect.hpp"
#include "window.hpp"
#include "rand_generator.hpp"

using namespace std::chrono;
using namespace std;

int main(int nargs, char *argv[])
{
    // DEFINITION DE CONSTANTES
    // on définit ici des constantes pour calculer les différentes étapes de l'algorithme et pour mesurer les temps d'exécution de chaque étape.
    double time_ants = 0, time_phen = 0, time_render = 0;
    int frame_count = 0;
    SDL_Init(SDL_INIT_VIDEO);
    const int nb_ants = 5000; // Nombre de fourmis
    const double eps = 0.8;     // Coefficient d'exploration
    const double alpha = 0.7;   // Coefficient de chaos
    // const double beta=0.9999; // Coefficient d'évaporation
    const double beta = 0.999; // Coefficient d'évaporation
    // Location du nid
    position_t pos_nest{256, 256};
    // Location de la nourriture
    position_t pos_food{500, 500};
    // const int i_food = 500, j_food = 500;

    // TERRITOIRE :
    //  Génération du TERRITOIRE 512 x 512 ( 2*(2^8) par direction )
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

    // FOURMIS :
    // Définition du coefficient d'exploration de toutes les FOURMIS.

    // On va créer des fourmis un peu partout sur la carte :
    VectorOfAnts theAnts(eps, nb_ants);
    theAnts.gen_Ants_pos(land);

    // On crée toutes les fourmis dans la fourmilière.
    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);
    Window win("Ant Simulation", 2 * land.dimensions() + 10, land.dimensions() + 266);
    RendererVect renderer(land, phen, pos_nest, pos_food, theAnts);

    // Compteur de la quantité de nourriture apportée au nid par les fourmis
    size_t food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;

    while (cont_loop)
    {
        ++it;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                cont_loop = false;
        }

        // avancement des fourmis
        auto t1 = steady_clock::now();
        advance_Vect(theAnts, phen, land, pos_food, pos_nest, food_quantity);
        auto t2 = steady_clock::now();

        // mise a jour phéromones
        phen.do_evaporation();
        phen.update();
        auto t3 = steady_clock::now();

        // rendu graphique
        renderer.display(win, food_quantity);
        win.blit();
        auto t4 = steady_clock::now();

        // calcul des temps d'exec
        time_ants += duration_cast<microseconds>(t2 - t1).count();
        time_phen += duration_cast<microseconds>(t3 - t2).count();
        time_render += duration_cast<microseconds>(t4 - t3).count();
        frame_count++;

        // affichage des stats toutes les 100 frames
        if (frame_count == 100)
        {
            std::cout << "-- Stats de durée sur 100 itérations --" << std::endl;
            std::cout << "Mouvement Fourmis : " << (time_ants / 100.0) / 1000.0 << " ms" << std::endl;
            std::cout << "Phéromones (Evap/Upd): " << (time_phen / 100.0) / 1000.0 << " ms" << std::endl;
            std::cout << "Rendu SDL : " << (time_render / 100.0) / 1000.0 << " ms" << std::endl;
            std::cout << "Total : " << ((time_ants + time_phen + time_render) / 100.0) / 1000.0 << " ms" << std::endl;

            // Reset
            time_ants = time_phen = time_render = 0;
            frame_count = 0;
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