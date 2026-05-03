#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <mpi.h>

#define INF INT_MAX
#define SYNC_INTERVAL 50

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

Graph* loadGraph(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Error: Cannot open %s\n", filename);
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

void printResults(long long* dist, int V) {
    printf("\n  Shortest distances from vertex 0:\n");
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
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0)
            printf("Usage: mpirun -np <procs> %s <graph_file>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int V = 0, E = 0;
    Graph* graph = NULL;

    if (rank == 0) {
        graph = loadGraph(argv[1]);
        if (!graph) MPI_Abort(MPI_COMM_WORLD, 1);
        V = graph->V;
        E = graph->E;

        printf("========================================\n");
        printf("  Bellman-Ford MPI Implementation\n");
        printf("========================================\n");
        printf("\nLoading graph from: %s\n", argv[1]);
        printf("  Vertices:      %d\n", V);
        printf("  Edges:         %d\n", E);
        printf("  Processes:     %d\n", size);
        printf("  Sync interval: every %d iterations\n", SYNC_INTERVAL);
        printf("  Running all %d iterations\n", V - 1);
        fflush(stdout);
    }

    MPI_Bcast(&V, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&E, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Datatype MPI_EDGE;
    int blocklengths[3] = {1, 1, 1};
    MPI_Aint displacements[3];
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    Edge dummy;
    MPI_Aint base_addr;
    MPI_Get_address(&dummy,        &base_addr);
    MPI_Get_address(&dummy.src,    &displacements[0]);
    MPI_Get_address(&dummy.dest,   &displacements[1]);
    MPI_Get_address(&dummy.weight, &displacements[2]);
    displacements[0] -= base_addr;
    displacements[1] -= base_addr;
    displacements[2] -= base_addr;
    MPI_Type_create_struct(3, blocklengths, displacements,
                           types, &MPI_EDGE);
    MPI_Type_commit(&MPI_EDGE);

    if (rank != 0)
        graph = createGraph(V, E);

    MPI_Bcast(graph->edges, E, MPI_EDGE, 0, MPI_COMM_WORLD);

    int chunk = E / size;
    int start_edge = rank * chunk;
    int end_edge = (rank == size - 1) ? E : start_edge + chunk;

    printf("  [DEBUG] Process %d handles edges %d to %d (%d edges)\n",
           rank, start_edge, end_edge, end_edge - start_edge);
    fflush(stdout);

    long long* dist       = (long long*)malloc(V * sizeof(long long));
    long long* local_dist = (long long*)malloc(V * sizeof(long long));

    for (int i = 0; i < V; i++)
        dist[i] = INF;
    dist[0] = 0;

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    for (int iter = 0; iter < V - 1; iter++) {
        for (int i = 0; i < V; i++)
            local_dist[i] = dist[i];

        for (int j = start_edge; j < end_edge; j++) {
            int u = graph->edges[j].src;
            int v = graph->edges[j].dest;
            int w = graph->edges[j].weight;
            if (local_dist[u] != INF && local_dist[u] + w < local_dist[v])
                local_dist[v] = local_dist[u] + w;
        }

        if ((iter + 1) % SYNC_INTERVAL == 0 || iter == V - 2) {
            MPI_Allreduce(local_dist, dist, V,
                         MPI_LONG_LONG, MPI_MIN, MPI_COMM_WORLD);
        } else {
            for (int i = 0; i < V; i++)
                if (local_dist[i] < dist[i])
                    dist[i] = local_dist[i];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double elapsed = MPI_Wtime() - start_time;

    if (rank == 0) {
        int neg_cycle = 0;
        for (int j = 0; j < E; j++) {
            int u = graph->edges[j].src;
            int v = graph->edges[j].dest;
            int w = graph->edges[j].weight;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                neg_cycle = 1;
                break;
            }
        }

        printf("\n--- Timing Results ---\n");
        printf("  Processes:      %d\n", size);
        printf("  Sync interval:  every %d iters\n", SYNC_INTERVAL);
        printf("  Execution time: %.6f seconds\n", elapsed);
        printf("  Execution time: %.3f milliseconds\n", elapsed * 1000);
        printf("  Status: %s\n", neg_cycle ?
               "NEGATIVE CYCLE DETECTED" : "SUCCESS (no negative cycle)");

        printResults(dist, V);

        FILE* rf = fopen("results/mpi_results.txt", "a");
        if (rf) {
            fprintf(rf, "V=%d E=%d processes=%d time=%.6f sec\n",
                    V, E, size, elapsed);
            fclose(rf);
            printf("\nTiming saved to results/mpi_results.txt\n");
        }
        printf("========================================\n");
    }

    MPI_Type_free(&MPI_EDGE);
    free(dist);
    free(local_dist);
    freeGraph(graph);
    MPI_Finalize();
    return 0;
}
CEOF
