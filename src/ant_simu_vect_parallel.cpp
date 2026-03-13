#include <mpi.h>
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

int main(int nargs, char *argv[])
{
    // initialisation de MPI
    MPI_Init(&nargs, &argv);
    int rank, nbp;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nbp);

    // DEFINITION DE CONSTANTES
    // on définit ici des constantes pour calculer les différentes étapes de l'algorithme et pour mesurer les temps d'exécution de chaque étape.
    double time_ants = 0, time_phen = 0, time_render = 0;
    int frame_count = 0;

    if (rank == 0)
    {
        SDL_Init(SDL_INIT_VIDEO);
    }

    const int nb_ants = 100000; // Nombre de fourmis
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
    // On va créer des fourmis un peu partout sur la carte, chaque processus créé toutes les fourmis avant de se les séparer
    VectorOfAnts allAnts(eps, nb_ants);
    allAnts.gen_Ants_pos(land);

    int ants_per_process = nb_ants / nbp;
    int start_ant = rank * ants_per_process;
    int end_ant = (rank == nbp - 1) ? nb_ants : start_ant + ants_per_process;
    int local_n = end_ant - start_ant;

    // chaque processus crée un vecteur de fourmis local qui contient les fourmis qui lui sont assignées
    VectorOfAnts theAnts(eps, local_n);
    for (int i = 0; i < local_n; ++i)
    {
        theAnts.pos_x[i] = allAnts.pos_x[start_ant + i];
        theAnts.pos_y[i] = allAnts.pos_y[start_ant + i];
        theAnts.states[i] = allAnts.states[start_ant + i];
        theAnts.seeds[i] = allAnts.seeds[start_ant + i];
        theAnts.consumed_time[i] = allAnts.consumed_time[start_ant + i];
    }

    // On crée toutes les fourmis dans la fourmilière.
    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    int pace = land.dimensions() + 2;
    int phen_size = pace * pace * 2;
    std::vector<double> local_buf(phen_size), global_buf(phen_size);

    Window *win = nullptr;
    RendererVect *renderer = nullptr;
    if (rank == 0)
    {
        win = new Window("Ant Simulation", 2 * land.dimensions() + 10, land.dimensions() + 266);
        renderer = new RendererVect(land, phen, pos_nest, pos_food, theAnts);
    }

    // Compteur de la quantité de nourriture apportée au nid par les fourmis
    size_t food_quantity = 0;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;

    while (cont_loop)
    {
        ++it;
        if (rank == 0)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
                if (event.type == SDL_QUIT)
                    cont_loop = false;
        }
        // broadcast de la condition d'arrêt à tous les processus pour sortir de la boucle si besoin
        MPI_Bcast(&cont_loop, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);

        if (!cont_loop)
            break;

        // avancement des fourmis
        auto t1 = steady_clock::now();
        size_t local_food = 0;
        advance_Vect(theAnts, phen, land, pos_food, pos_nest, local_food);
        auto t2 = steady_clock::now();

        size_t global_food = 0;
        MPI_Allreduce(&local_food, &global_food, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);
        food_quantity += global_food;

        // phéromones : on trouve le max de tous les processus, le mettons dans buffer local
        auto t3 = steady_clock::now();
        {
            auto &raw = phen;
            int idx = 0;
            for (int i = -1; i <= (int)land.dimensions(); ++i)
                for (int j = -1; j <= (int)land.dimensions(); ++j)
                {
                    // dimension réélle de la carte sans les bords fantômes
                    if (i >= 0 && i < (int)land.dimensions() &&
                        j >= 0 && j < (int)land.dimensions())
                    {
                        local_buf[idx++] = raw(i, j)[0];
                        local_buf[idx++] = raw(i, j)[1];
                    }
                    // bords fantomes : -1 pour eviter que les fourmis aillent sur ces cases
                    else
                    {
                        local_buf[idx++] = -1.;
                        local_buf[idx++] = -1.;
                    }
                }
        }

        // all reduce permet de recupereller le max de tous les processus dans global_buf
        MPI_Allreduce(local_buf.data(), global_buf.data(), phen_size,
                      MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        // on reinjecte les valeurs max dans la carte des phéromones de chacun des processus
        {
            int idx = 0;
            for (int i = -1; i <= (int)land.dimensions(); ++i)
                for (int j = -1; j <= (int)land.dimensions(); ++j)
                {
                    if (i >= 0 && i < (int)land.dimensions() &&
                        j >= 0 && j < (int)land.dimensions())
                    {
                        phen(i, j)[0] = global_buf[idx++];
                        phen(i, j)[1] = global_buf[idx++];
                    }
                    // on saute les bords fantômes
                    else
                    {
                        idx += 2;
                    }
                }
        }

        // retour aux fonctions de base
        phen.do_evaporation_mpi();
        phen.update();
        auto t4 = steady_clock::now();

        // seulement le processus 0 effectue le rendu graphique
        auto t5 = steady_clock::now();
        if (rank == 0)
        {
            renderer->display(*win, food_quantity);
            win->blit();
        }
        auto t6 = steady_clock::now();

        // calcul des temps d'exec de chaque étape comme à l'origine
        time_ants += duration_cast<microseconds>(t2 - t1).count();
        time_phen += duration_cast<microseconds>(t4 - t3).count();
        time_render += duration_cast<microseconds>(t6 - t5).count();
        frame_count++;

        if (frame_count == 100 && rank == 0)
        {
            std::cout << "-- Stats sur 100 itérations (rang 0) --" << std::endl;
            std::cout << "Mouvement Fourmis    : " << (time_ants / 100.0) / 1000.0 << " ms" << std::endl;
            std::cout << "Phéromones(avec MPI) : " << (time_phen / 100.0) / 1000.0 << " ms" << std::endl;
            std::cout << "Rendu SDL            : " << (time_render / 100.0) / 1000.0 << " ms" << std::endl;
            std::cout << "Total                : " << ((time_ants + time_phen + time_render) / 100.0) / 1000.0 << " ms" << std::endl;
            time_ants = time_phen = time_render = 0;
            frame_count = 0;
        }

        if (not_food_in_nest && food_quantity > 0)
        {
            if (rank == 0)
                std::cout << "Première nourriture au nid à l'itération " << it << std::endl;
            not_food_in_nest = false;
        }
    }

    if (rank == 0)
    {
        delete renderer;
        delete win;
        SDL_Quit();
    }

    MPI_Finalize();
    return 0;
}