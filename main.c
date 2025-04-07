/*
    Projeto: Sistema de Aeroporto até o Embarque
    Descrição: Simulação de criação e execução de threads representando agentes/workers de um aeroporto.
    Etapa: Sincronização de Threads.

    Integrantes do Grupo:
        - Guilherme Teodoro de Oliveira  RA: 10425362
        - Luís Henrique Ribeiro Fernandes  RA: 10420079
        - Vinícius Brait Lorimier  RA: 10420046
*/

// Declaração das bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_THREADS 5

typedef struct {
    int id;
    char* name;
} argsThreads;

// Variáveis externas definidas em airport.c
extern int passengers_processed;
extern int checked_baggage;
extern int status_steps[MAX_THREADS];
extern int gate_open;

// Funções implementadas em airport.c
void initialize_synchronization();
void cleanup_synchronization();
void print_simulation_summary(argsThreads args[]);
void* worker_airport(void* arguments);

int main() {
    pthread_t threads[MAX_THREADS];
    argsThreads args[MAX_THREADS];
    
    printf("Simulação de Threads Sincronizadas: Sistema de Aeroporto\n");
    
    // Inicializa mutexes e variáveis de condição
    initialize_synchronization();
    
    // Criação das threads (workers)
    for (int i = 0; i < MAX_THREADS; i++) {
        args[i].id = i;
        
        // Definine nomes dos workers
        switch (i) {
            case 0:
                args[i].name = "Embarque";
                break;
            case 1:
                args[i].name = "Bagagem";
                break;
            case 2:
                args[i].name = "Segurança";
                break;
            case 3:
                args[i].name = "Check-in";
                break;
            case 4:
                args[i].name = "Portão";
                break;
        }
        
        // Criação da thread
        if (pthread_create(&threads[i], NULL, worker_airport, (void*)&args[i]) != 0) {
            perror("Erro ao criar thread");
            exit(EXIT_FAILURE);
        }
    }
    
    // Finalização de todas as threads
    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Erro ao aguardar thread");
            exit(EXIT_FAILURE);
        }
    }
    
    // Limpa recursos de sincronização
    cleanup_synchronization();
    // Exibe o resumo da simulação e finaliza o programa
    print_simulation_summary(args);
    
    return 0;
}