import matplotlib.pyplot as plt
import numpy as np
# Données extraites de votre tableau
# Données (Séquentiel en premier pour le calcul)
t_seq = [1.54553, 0.36467, 27.9393, 29.8495] # Mouv, Phero, SDL, Total

threads = np.array([1, 2, 4, 8, 10, 12, 13, 14, 20, 24, 64])
mouvement = np.array([1.44103, 1.31456, 1.28750, 1.47684, 2.04603 ,3.20630, 0.94152 ,0.98006 ,0.99227,0.97441, 1.07474])
pheromones = np.array([0.29182, 0.29115, 0.21275, 0.26395, 0.22434, 2.06319,0.45644 ,0.55096, 0.5715,0.55628, 0.90218])
rendu_sdl = np.array([27.7129, 27.7540, 27.9082, 29.1860, 27.1413, 23.5096, 29.5267 ,28.2939, 28.3047, 27.2454, 26.7957])
total = np.array([29.4458, 29.3597, 29.4084, 30.9268,29.4116, 28.7791, 30.9247, 28.658, 29.8685, 28.7760, 28.7726])

# Calcul du Speedup S(n) = T_seq / T(n)
speedup_mouv = t_seq[0] / mouvement
speedup_phero = t_seq[1] / pheromones
speedup_total = t_seq[3] / total

plt.figure(figsize=(10, 6))

# Tracé des courbes
plt.plot(threads, mouvement, marker='o', label='Mouvement Fourmis')
plt.plot(threads, pheromones, marker='s', label='Phéromones (Evap/Upd)')

# Mise en forme
plt.xscale('log') # Échelle logarithmique pour mieux voir les petits nombres de threads
plt.xticks(threads, labels=threads)
plt.xlabel('Nombre de threads (échelle log)')
plt.ylabel('Temps (ms)')
plt.title('Performance de la simulation en fonction du parallélisme (OpenMP)')
plt.grid(True, which="both", ls="-", alpha=0.5)
plt.legend()

# Annotation de la zone critique (12 threads)
plt.annotate('Chute de performance (E-cores)', xy=(12, 3.2), xytext=(2,3),
             arrowprops=dict(facecolor='red', shrink=0.05))

plt.tight_layout()
plt.show()

plt.plot(threads, rendu_sdl, marker='^', label='Rendu SDL')
plt.plot(threads, total, marker='x', linestyle='--', color='black', label='Total')

# Mise en forme
plt.xscale('log') # Échelle logarithmique pour mieux voir les petits nombres de threads
plt.xticks(threads, labels=threads)
plt.xlabel('Nombre de threads (échelle log)')
plt.ylabel('Temps (ms)')
plt.title('Performance de la simulation en fonction du parallélisme (OpenMP)')
plt.grid(True, which="both", ls="-", alpha=0.5)
plt.legend()


plt.tight_layout()
plt.show()

# Tracé du Speedup
plt.figure(figsize=(10, 6))

plt.plot(threads, speedup_mouv, 'o-', label='Speedup Mouvement')
plt.plot(threads, speedup_phero, 's-', label='Speedup Phéromones')
plt.plot(threads, speedup_total, 'x--', color='black', label='Speedup Global')

plt.xscale('log', base=2)
plt.yscale('linear')
plt.xlabel('Nombre de threads')
plt.ylabel('Valeur du Speedup')
plt.title('Analyse du Speedup par phase de calcul')
plt.legend()
plt.grid(True, which="both", ls="-", alpha=0.3)

plt.tight_layout()
plt.show()