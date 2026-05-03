

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <omp.h>

#define INF INT_MAX

typedef struct {
    int src, dest, weight;
} Edge;

typedef struct {
    int V, E;
    Edge* edges;
} Graph;

Graph* createGraph(int V, int E) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->V = V;
    graph->E = E;
    graph->edges = (Edge*)malloc(E * sizeof(Edge));
    return graph;
}

void freeGraph(Graph* graph) {
    free(graph->edges);
    free(graph);
}

double getTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int bellmanFordOpenMP(Graph* graph, int src, long long* dist, int num_threads) {
    int V = graph->V;
    int E = graph->E;
    Edge* edges = graph->edges;

    for (int i = 0; i < V; i++)
        dist[i] = INF;
    dist[src] = 0;

    omp_set_num_threads(num_threads);

    for (int iter = 0; iter < V - 1; iter++) {
        #pragma omp parallel for schedule(dynamic, 256)
        for (int j = 0; j < E; j++) {
            int u = edges[j].src;
            int v = edges[j].dest;
            int w = edges[j].weight;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                #pragma omp critical
                {
                    if (dist[u] != INF && dist[u] + w < dist[v])
                        dist[v] = dist[u] + w;
                }
            }
        }
    }

    for (int j = 0; j < E; j++) {
        int u = edges[j].src;
        int v = edges[j].dest;
        int w = edges[j].weight;
        if (dist[u] != INF && dist[u] + w < dist[v])
            return 0;
    }
    return 1;
}

Graph* loadGraph(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    int V, E;
    fscanf(f, "%d %d", &V, &E);
    Graph* graph = createGraph(V, E);
    for (int i = 0; i < E; i++)
        fscanf(f, "%d %d %d",
               &graph->edges[i].src,
               &graph->edges[i].dest,
               &graph->edges[i].weight);
    fclose(f);
    return graph;
}

void printResults(long long* dist, int V, int src) {
    printf("\n  Shortest distances from vertex %d:\n", src);
    int limit = V < 10 ? V : 10;
    for (int i = 0; i < limit; i++) {
        if (dist[i] == INF)
            printf("    dist[%d] = INF\n", i);
        else
            printf("    dist[%d] = %lld\n", i, dist[i]);
    }
    if (V > 10) printf("    ... (showing first 10 of %d)\n", V);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <graph_file> [num_threads]\n", argv[0]);
        return 1;
    }

    int num_threads = (argc >= 3) ? atoi(argv[2]) : omp_get_max_threads();

    printf("========================================\n");
    printf("  Bellman-Ford OpenMP Implementation\n");
    printf("========================================\n");

    printf("\nLoading graph from: %s\n", argv[1]);
    Graph* graph = loadGraph(argv[1]);
    if (!graph) return 1;

    printf("  Vertices: %d\n", graph->V);
    printf("  Edges:    %d\n", graph->E);
    printf("  Threads:  %d\n", num_threads);

    long long* dist = (long long*)malloc(graph->V * sizeof(long long));
    int src = 0;

    printf("\nRunning OpenMP Bellman-Ford from source vertex %d...\n", src);
    printf("  Running all %d iterations (benchmarking mode)\n", graph->V - 1);

    double start = getTime();
    int result = bellmanFordOpenMP(graph, src, dist, num_threads);
    double end = getTime();
    double elapsed = end - start;

    if (result)
        printf("  Status: SUCCESS (no negative cycle)\n");
    else
        printf("  Status: NEGATIVE CYCLE DETECTED\n");

    printf("\n--- Timing Results ---\n");
    printf("  Threads:        %d\n", num_threads);
    printf("  Execution time: %.6f seconds\n", elapsed);
    printf("  Execution time: %.3f milliseconds\n", elapsed * 1000);

    printResults(dist, graph->V, src);

    FILE* rf = fopen("results/openmp_results.txt", "a");
    if (rf) {
        fprintf(rf, "V=%d E=%d threads=%d time=%.6f sec\n",
                graph->V, graph->E, num_threads, elapsed);
        fclose(rf);
        printf("\nTiming saved to results/openmp_results.txt\n");
    }

    free(dist);
    freeGraph(graph);
    printf("========================================\n");
    return 0;
}
CEOF
