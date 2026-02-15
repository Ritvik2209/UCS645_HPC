#include <iostream>
#include <cmath>
#include <omp.h>
#include <iomanip>
#include <cstdlib>
#include <cstring>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <grid_size> [max_threads]\n";
        cout << "Example: " << argv[0] << " 500 8\n";
        return 1;
    }
    
    int N = atoi(argv[1]);
    if (N <= 0) {
        cerr << "Error: Grid size must be positive\n";
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
    
    const double alpha = 0.01;
    const double dx = 0.1;
    const double dt = 0.001;
    const double total_time = 1.0;
    const int time_steps = (int)(total_time / dt);
    const double r = alpha * dt / (dx * dx);
    
    double** T = new double*[N];
    double** T_new = new double*[N];
    for (int i = 0; i < N; i++) {
        T[i] = new double[N];
        T_new[i] = new double[N];
    }
    
    int center = N / 2;
    int radius = N / 10;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double dist = sqrt((i - center) * (i - center) + (j - center) * (j - center));
            T[i][j] = (dist < radius) ? 100.0 : 20.0;
            T_new[i][j] = T[i][j];
        }
    }
    
    for (int i = 0; i < N; i++) {
        T[i][0] = T[i][N-1] = 20.0;
        T[0][i] = T[N-1][i] = 20.0;
    }
    
    cout << "Heat Diffusion Simulation (2D)\n";
    cout << "Grid size: " << N << " x " << N << "\n";
    cout << "Time steps: " << time_steps << "\n";
    cout << "Max threads: " << omp_get_max_threads() << "\n";
    cout << string(70, '=') << "\n\n";
    
    const char* schedules[] = {"static", "dynamic", "guided"};
    int num_schedules = 3;
    
    int* threads = new int[10];
    int count = 0;
    for (int t = 1; t <= max_threads && count < 10; t = (t == 1 ? 2 : t * 2)) {
        threads[count++] = t;
    }
    
    for (int sched_idx = 0; sched_idx < num_schedules; sched_idx++) {
        const char* schedule = schedules[sched_idx];
        
        cout << "\n=== Schedule: " << schedule << " ===\n";
        cout << left << setw(10) << "Threads" 
             << setw(15) << "Time (s)" 
             << setw(12) << "Speedup" 
             << setw(15) << "Efficiency %" << "\n";
        cout << string(55, '-') << "\n";
        
        double serial_time = 0.0;
        
        for (int idx = 0; idx < count; idx++) {
            int num_threads = threads[idx];
            omp_set_num_threads(num_threads);
            
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    double dist = sqrt((i - center) * (i - center) + (j - center) * (j - center));
                    T[i][j] = (dist < radius) ? 100.0 : 20.0;
                }
            }
            
            double start = omp_get_wtime();
            
            for (int t = 0; t < time_steps; t++) {
                if (num_threads == 1) {
                    for (int i = 1; i < N-1; i++) {
                        for (int j = 1; j < N-1; j++) {
                            T_new[i][j] = T[i][j] + r * (
                                T[i+1][j] + T[i-1][j] + 
                                T[i][j+1] + T[i][j-1] - 4.0 * T[i][j]
                            );
                        }
                    }
                } else {
                    if (strcmp(schedule, "static") == 0) {
                        #pragma omp parallel for schedule(static) collapse(2)
                        for (int i = 1; i < N-1; i++) {
                            for (int j = 1; j < N-1; j++) {
                                T_new[i][j] = T[i][j] + r * (
                                    T[i+1][j] + T[i-1][j] + 
                                    T[i][j+1] + T[i][j-1] - 4.0 * T[i][j]
                                );
                            }
                        }
                    } else if (strcmp(schedule, "dynamic") == 0) {
                        #pragma omp parallel for schedule(dynamic, 10) collapse(2)
                        for (int i = 1; i < N-1; i++) {
                            for (int j = 1; j < N-1; j++) {
                                T_new[i][j] = T[i][j] + r * (
                                    T[i+1][j] + T[i-1][j] + 
                                    T[i][j+1] + T[i][j-1] - 4.0 * T[i][j]
                                );
                            }
                        }
                    } else {
                        #pragma omp parallel for schedule(guided) collapse(2)
                        for (int i = 1; i < N-1; i++) {
                            for (int j = 1; j < N-1; j++) {
                                T_new[i][j] = T[i][j] + r * (
                                    T[i+1][j] + T[i-1][j] + 
                                    T[i][j+1] + T[i][j-1] - 4.0 * T[i][j]
                                );
                            }
                        }
                    }
                }
                
                double** temp = T;
                T = T_new;
                T_new = temp;
            }
            
            double end = omp_get_wtime();
            double elapsed = end - start;
            
            if (num_threads == 1) {
                serial_time = elapsed;
            }
            
            double speedup = (num_threads == 1) ? 1.0 : serial_time / elapsed;
            double efficiency = speedup / num_threads;
            
            cout << setw(10) << num_threads
                 << setw(15) << fixed << setprecision(6) << elapsed
                 << setw(12) << setprecision(2) << speedup
                 << setw(15) << setprecision(2) << (efficiency * 100) << "\n";
        }
    }
    
    for (int i = 0; i < N; i++) {
        delete[] T[i];
        delete[] T_new[i];
    }
    delete[] T;
    delete[] T_new;
    delete[] threads;
    
    return 0;
}

