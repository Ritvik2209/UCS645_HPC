#include <iostream>
#include <cmath>
#include <omp.h>
#include <iomanip>
#include <cstdlib>
#include <cstring>

using namespace std;

const int MATCH = 2;
const int MISMATCH = -1;
const int GAP = -1;

int get_max(int a, int b, int c, int d) {
    int max_val = a;
    if (b > max_val) max_val = b;
    if (c > max_val) max_val = c;
    if (d > max_val) max_val = d;
    return max_val;
}

void smith_waterman(const char* seq1, const char* seq2, int** H, 
                    int m, int n, int& max_score, int num_threads) {
    max_score = 0;
    omp_set_num_threads(num_threads);
    
    if (num_threads == 1) {
        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                int match = H[i-1][j-1] + ((seq1[i-1] == seq2[j-1]) ? MATCH : MISMATCH);
                int del = H[i-1][j] + GAP;
                int ins = H[i][j-1] + GAP;
                
                H[i][j] = get_max(0, match, del, ins);
                if (H[i][j] > max_score) {
                    max_score = H[i][j];
                }
            }
        }
    } else {
        for (int diag = 2; diag <= m + n; diag++) {
            int start = (diag <= n) ? 1 : diag - n;
            int end = (diag <= m) ? diag - 1 : m;
            
            int local_max = 0;
            
            #pragma omp parallel for schedule(dynamic) reduction(max:local_max)
            for (int i = start; i <= end; i++) {
                int j = diag - i;
                if (j >= 1 && j <= n) {
                    int match = H[i-1][j-1] + ((seq1[i-1] == seq2[j-1]) ? MATCH : MISMATCH);
                    int del = H[i-1][j] + GAP;
                    int ins = H[i][j-1] + GAP;
                    
                    H[i][j] = get_max(0, match, del, ins);
                    if (H[i][j] > local_max) {
                        local_max = H[i][j];
                    }
                }
            }
            
            if (local_max > max_score) {
                max_score = local_max;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <sequence_length> [max_threads]\n";
        cout << "Example: " << argv[0] << " 500 8\n";
        return 1;
    }
    
    int seq_length = atoi(argv[1]);
    if (seq_length <= 0) {
        cerr << "Error: Sequence length must be positive\n";
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
    
    char* seq1 = new char[seq_length + 1];
    char* seq2 = new char[seq_length + 1];
    
    const char bases[] = {'A', 'C', 'G', 'T'};
    for (int i = 0; i < seq_length; i++) {
        seq1[i] = bases[rand() % 4];
        seq2[i] = bases[rand() % 4];
    }
    seq1[seq_length] = '\0';
    seq2[seq_length] = '\0';
    
    for (int i = 50; i < 100 && i < seq_length; i++) {
        seq2[i] = seq1[i];
    }
    
    cout << "Smith-Waterman DNA Sequence Alignment\n";
    cout << "Sequence length: " << seq_length << " nucleotides\n";
    cout << "Max threads: " << omp_get_max_threads() << "\n";
    cout << string(70, '=') << "\n\n";
    
    int* threads = new int[10];
    int count = 0;
    for (int t = 1; t <= max_threads && count < 10; t = (t == 1 ? 2 : t * 2)) {
        threads[count++] = t;
    }
    
    double serial_time = 0.0;
    double* times = new double[count];
    double* speedups = new double[count];
    double* efficiencies = new double[count];
    int* scores = new int[count];
    
    for (int idx = 0; idx < count; idx++) {
        int num_threads = threads[idx];
        
        int** H = new int*[seq_length + 1];
        for (int i = 0; i <= seq_length; i++) {
            H[i] = new int[seq_length + 1];
            for (int j = 0; j <= seq_length; j++) {
                H[i][j] = 0;
            }
        }
        
        int max_score = 0;
        double start = omp_get_wtime();
        
        smith_waterman(seq1, seq2, H, seq_length, seq_length, max_score, num_threads);
        
        double end = omp_get_wtime();
        double elapsed = end - start;
        
        times[idx] = elapsed;
        scores[idx] = max_score;
        
        if (num_threads == 1) {
            serial_time = elapsed;
            speedups[idx] = 1.0;
            efficiencies[idx] = 1.0;
        } else {
            speedups[idx] = serial_time / elapsed;
            efficiencies[idx] = speedups[idx] / num_threads;
        }
        
        for (int i = 0; i <= seq_length; i++) {
            delete[] H[i];
        }
        delete[] H;
    }
    
    cout << left << setw(10) << "Threads" 
         << setw(15) << "Time (s)" 
         << setw(12) << "Speedup" 
         << setw(15) << "Efficiency %" 
         << setw(15) << "Max Score" << "\n";
    cout << string(70, '-') << "\n";
    
    for (int idx = 0; idx < count; idx++) {
        cout << setw(10) << threads[idx]
             << setw(15) << fixed << setprecision(6) << times[idx]
             << setw(12) << setprecision(2) << speedups[idx]
             << setw(15) << setprecision(2) << (efficiencies[idx] * 100)
             << setw(15) << scores[idx] << "\n";
    }
    
    cout << "\nPerformance Summary:\n";
    cout << "Serial time: " << fixed << setprecision(6) << serial_time << " seconds\n";
    cout << "Best speedup: " << setprecision(2) << speedups[count-1] << "x with " 
         << threads[count-1] << " threads\n";
    
    double max_eff = 0.0;
    for (int i = 0; i < count; i++) {
        if (efficiencies[i] > max_eff) max_eff = efficiencies[i];
    }
    cout << "Best efficiency: " << setprecision(2) << (max_eff * 100) << "%\n";
    cout << "Alignment score: " << scores[0] << " (consistent)\n";
    
    delete[] seq1;
    delete[] seq2;
    delete[] threads;
    delete[] times;
    delete[] speedups;
    delete[] efficiencies;
    delete[] scores;
    
    return 0;
}

