#include <iostream>
#include <cmath>
#include <omp.h>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace std;

struct Particle {
    double x, y, z;
    double fx, fy, fz;
};

const double EPSILON = 1.0;
const double SIGMA = 1.0;
const double CUTOFF = 2.5 * SIGMA;

double random_double(double min, double max) {
    return min + (max - min) * rand() / (double)RAND_MAX;
}

void compute_LJ_force(const Particle& pi, const Particle& pj, 
                      double& fx, double& fy, double& fz, double& energy) {
    double dx = pj.x - pi.x;
    double dy = pj.y - pi.y;
    double dz = pj.z - pi.z;
    
    double r2 = dx*dx + dy*dy + dz*dz;
    double r = sqrt(r2);
    
    if (r > CUTOFF) {
        fx = fy = fz = energy = 0.0;
        return;
    }
    
    double sigma_r = SIGMA / r;
    double sigma_r6 = pow(sigma_r, 6);
    double sigma_r12 = sigma_r6 * sigma_r6;
    
    energy = 4.0 * EPSILON * (sigma_r12 - sigma_r6);
    double force_magnitude = 24.0 * EPSILON * (2.0 * sigma_r12 - sigma_r6) / r;
    
    fx = force_magnitude * dx / r;
    fy = force_magnitude * dy / r;
    fz = force_magnitude * dz / r;
}

void print_usage(const char* prog_name) {
    cout << "Usage: " << prog_name << " <num_particles> [max_threads]\n";
    cout << "Example: " << prog_name << " 1000 8\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    int N = atoi(argv[1]);
    if (N <= 0) {
        cerr << "Error: Number of particles must be positive\n";
        return 1;
    }
    
    int max_threads = omp_get_max_threads();
    if (argc >= 3) {
        max_threads = atoi(argv[2]);
        if (max_threads <= 0) {
            cerr << "Error: Number of threads must be positive\n";
            return 1;
        }
    }
    
    srand(42);
    
    Particle* particles = new Particle[N];
    
    for (int i = 0; i < N; i++) {
        particles[i].x = random_double(0.0, 10.0);
        particles[i].y = random_double(0.0, 10.0);
        particles[i].z = random_double(0.0, 10.0);
        particles[i].fx = particles[i].fy = particles[i].fz = 0.0;
    }
    
    cout << "Molecular Dynamics - Lennard-Jones Force Calculation\n";
    cout << "Number of particles: " << N << "\n";
    cout << "Max threads: " << omp_get_max_threads() << "\n";
    cout << string(70, '=') << "\n\n";
    
    int* thread_list = new int[10];
    int thread_count = 0;
    for (int t = 1; t <= max_threads && thread_count < 10; t = (t == 1 ? 2 : t * 2)) {
        thread_list[thread_count++] = t;
    }
    
    double serial_time = 0.0;
    double* times = new double[thread_count];
    double* speedups = new double[thread_count];
    double* efficiencies = new double[thread_count];
    double* energies = new double[thread_count];
    
    for (int idx = 0; idx < thread_count; idx++) {
        int num_threads = thread_list[idx];
        omp_set_num_threads(num_threads);
        
        for (int i = 0; i < N; i++) {
            particles[i].fx = particles[i].fy = particles[i].fz = 0.0;
        }
        
        double total_energy = 0.0;
        double start_time = omp_get_wtime();
        
        #pragma omp parallel reduction(+:total_energy)
        {
            #pragma omp for schedule(dynamic, 10)
            for (int i = 0; i < N; i++) {
                double local_fx = 0.0, local_fy = 0.0, local_fz = 0.0;
                
                for (int j = 0; j < N; j++) {
                    if (i != j) {
                        double fx, fy, fz, energy;
                        compute_LJ_force(particles[i], particles[j], fx, fy, fz, energy);
                        
                        local_fx += fx;
                        local_fy += fy;
                        local_fz += fz;
                        
                        if (i < j) {
                            total_energy += energy;
                        }
                    }
                }
                
                #pragma omp atomic
                particles[i].fx += local_fx;
                
                #pragma omp atomic
                particles[i].fy += local_fy;
                
                #pragma omp atomic
                particles[i].fz += local_fz;
            }
        }
        
        double end_time = omp_get_wtime();
        double elapsed = end_time - start_time;
        
        times[idx] = elapsed;
        energies[idx] = total_energy;
        
        if (num_threads == 1) {
            serial_time = elapsed;
            speedups[idx] = 1.0;
            efficiencies[idx] = 1.0;
        } else {
            speedups[idx] = serial_time / elapsed;
            efficiencies[idx] = speedups[idx] / num_threads;
        }
    }
    
    cout << left << setw(10) << "Threads" 
         << setw(15) << "Time (s)" 
         << setw(12) << "Speedup" 
         << setw(15) << "Efficiency %" 
         << setw(15) << "Energy" << "\n";
    cout << string(70, '-') << "\n";
    
    for (int idx = 0; idx < thread_count; idx++) {
        cout << setw(10) << thread_list[idx]
             << setw(15) << fixed << setprecision(6) << times[idx]
             << setw(12) << setprecision(2) << speedups[idx]
             << setw(15) << setprecision(2) << (efficiencies[idx] * 100)
             << setw(15) << setprecision(4) << energies[idx] << "\n";
    }
    
    cout << "\nPerformance Summary:\n";
    cout << "Serial time: " << fixed << setprecision(6) << serial_time << " seconds\n";
    cout << "Best speedup: " << setprecision(2) << speedups[thread_count-1] 
         << "x with " << thread_list[thread_count-1] << " threads\n";
    
    double max_eff = 0.0;
    for (int i = 0; i < thread_count; i++) {
        if (efficiencies[i] > max_eff) max_eff = efficiencies[i];
    }
    cout << "Best efficiency: " << setprecision(2) << (max_eff * 100) << "%\n";
    
    delete[] particles;
    delete[] thread_list;
    delete[] times;
    delete[] speedups;
    delete[] efficiencies;
    delete[] energies;
    
    return 0;
}

