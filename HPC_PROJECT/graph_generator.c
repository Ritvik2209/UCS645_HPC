#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <V> <E> <output_file> [seed]\n", argv[0]);
        return 1;
    }

    int V = atoi(argv[1]);
    int E = atoi(argv[2]);
    const char* filename = argv[3];
    int seed = (argc >= 5) ? atoi(argv[4]) : (int)time(NULL);

    int max_edges = V * (V - 1);
    if (E > max_edges) {
        printf("Warning: E=%d exceeds max %d for V=%d\n", E, max_edges, V);
        E = max_edges;
    }

    srand(seed);
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Error: Cannot create file %s\n", filename);
        return 1;
    }

    fprintf(f, "%d %d\n", V, E);

    int chain_edges = (E >= V - 1) ? V - 1 : E;
    for (int i = 0; i < chain_edges; i++) {
        int weight = (rand() % 100) + 1;
        fprintf(f, "%d %d %d\n", i, i + 1, weight);
    }

    int remaining = E - chain_edges;
    for (int i = 0; i < remaining; i++) {
        int u = rand() % V;
        int v = rand() % V;
        while (v == u) v = rand() % V;
        int weight = (rand() % 100) + 1;
        fprintf(f, "%d %d %d\n", u, v, weight);
    }

    fclose(f);
    printf("Graph generated!\n");
    printf("  File: %s\n  Vertices: %d\n  Edges: %d\n", filename, V, E);
    return 0;
}
CEOF
