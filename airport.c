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

// Quantidade máxima de threads (workers)
#define MAX_THREADS 5

// Variáveis compartilhadas (seções críticas)
int passengers_processed = 0;      
int checked_baggage = 0;          
int status_steps[MAX_THREADS] = {0};   
int gate_open = 0;                

// Mutex para proteger as seções críticas
pthread_mutex_t mutex_passengers;    
pthread_mutex_t mutex_baggage;       
pthread_mutex_t mutex_status;       
pthread_mutex_t mutex_gate;        

// Variáveis de condição
pthread_cond_t cond_checkin_completed;   
pthread_cond_t cond_security_completed; 
pthread_cond_t cond_baggage_completed;  
pthread_cond_t cond_gate_open;   

// Estrutura para armazenar os argumentos das threads
typedef struct {
    int id;
    char* name;
} argsThreads;

// Função para inicializar mutexes e variáveis de condição
void initialize_synchronization() {
    pthread_mutex_init(&mutex_passengers, NULL);
    pthread_mutex_init(&mutex_baggage, NULL);
    pthread_mutex_init(&mutex_status, NULL);
    pthread_mutex_init(&mutex_gate, NULL);
    
    pthread_cond_init(&cond_checkin_completed, NULL);
    pthread_cond_init(&cond_security_completed, NULL);
    pthread_cond_init(&cond_baggage_completed, NULL);
    pthread_cond_init(&cond_gate_open, NULL);
    
    printf("Mutexes e variáveis de condição inicializados.\n");
}

// Função para destruir mutexes e variáveis de condição
void cleanup_synchronization() {
    pthread_mutex_destroy(&mutex_passengers);
    pthread_mutex_destroy(&mutex_baggage);
    pthread_mutex_destroy(&mutex_status);
    pthread_mutex_destroy(&mutex_gate);
    
    pthread_cond_destroy(&cond_checkin_completed);
    pthread_cond_destroy(&cond_security_completed);
    pthread_cond_destroy(&cond_baggage_completed);
    pthread_cond_destroy(&cond_gate_open);
}

// Função executada por cada thread (worker do aeroporto)
void* worker_airport(void* arguments) {
    argsThreads* args = (argsThreads*)arguments;
    int id = args->id;
    
    // Início da thread
    printf("Thread %d (Worker: %s) iniciada.\n", id, args->name);
    
    switch (id) {
        case 0: // Embarque
            // Esperar pela liberação do portão
            pthread_mutex_lock(&mutex_gate);
            while (gate_open == 0) {
                printf("Thread %d (Worker: %s) aguardando liberação do portão...\n", id, args->name);
                pthread_cond_wait(&cond_gate_open, &mutex_gate);
            }
            pthread_mutex_unlock(&mutex_gate);
            
            // Realizar o embarque
            printf("Thread %d (Worker: %s) iniciando embarque de passageiros...\n", id, args->name);
            sleep(2); // Simulando o tempo de embarque
            
            // Atualizar status da etapa
            pthread_mutex_lock(&mutex_status);
            status_steps[id] = 1;
            pthread_mutex_unlock(&mutex_status);
            
            printf("Thread %d (Worker: %s) concluiu o embarque de passageiros.\n", id, args->name);
            break;
            
        case 1: // Bagagem
            // Processar bagagens
            printf("Thread %d (Worker: %s) verificando bagagens...\n", id, args->name);
            sleep(3); // Simulando o tempo de verificação de bagagens
            
            // Atualizar o contador de bagagens (seção crítica)
            pthread_mutex_lock(&mutex_baggage);
            checked_baggage += 10; // Processa 10 bagagens
            printf("Thread %d (Worker: %s) verificou %d bagagens no total.\n", id, args->name, checked_baggage);
            pthread_mutex_unlock(&mutex_baggage);
            
            // Atualizar status da etapa
            pthread_mutex_lock(&mutex_status);
            status_steps[id] = 1;
            pthread_mutex_unlock(&mutex_status);
            
            // Sinalizar que a verificação de bagagens foi concluída
            pthread_cond_signal(&cond_baggage_completed);
            printf("Thread %d (Worker: %s) concluiu verificação de bagagens.\n", id, args->name);
            break;
            
        case 2: // Segurança
            // Esperar pela conclusão do check-in
            pthread_mutex_lock(&mutex_status);
            while (status_steps[3] == 0) { // Aguarda o check-in (ID 3) ser concluído
                printf("Thread %d (Worker: %s) aguardando conclusão do check-in...\n", id, args->name);
                pthread_cond_wait(&cond_checkin_completed, &mutex_status);
            }
            pthread_mutex_unlock(&mutex_status);
            
            // Realizar a verificação de segurança
            printf("Thread %d (Worker: %s) realizando verificação de segurança...\n", id, args->name);
            sleep(2); // Simulando o tempo de verificação de segurança
            
            // Atualizar status da etapa
            pthread_mutex_lock(&mutex_status);
            status_steps[id] = 1;
            pthread_mutex_unlock(&mutex_status);
            
            // Sinalizar que a segurança foi concluída
            pthread_cond_signal(&cond_security_completed);
            printf("Thread %d (Worker: %s) concluiu verificação de segurança.\n", id, args->name);
            break;
            
        case 3: // Check-in
            // Realizar o check-in
            printf("Thread %d (Worker: %s) processando check-in de passageiros...\n", id, args->name);
            sleep(2); // Simulando o tempo de check-in
            
            // Atualizar o contador de passageiros (seção crítica)
            pthread_mutex_lock(&mutex_passengers);
            passengers_processed += 20; // Processa 20 passageiros
            printf("Thread %d (Worker: %s) processou %d passageiros no total.\n", id, args->name, passengers_processed);
            pthread_mutex_unlock(&mutex_passengers);
            
            // Atualizar status da etapa
            pthread_mutex_lock(&mutex_status);
            status_steps[id] = 1;
            pthread_mutex_unlock(&mutex_status);
            
            // Sinalizar que o check-in foi concluído
            pthread_cond_signal(&cond_checkin_completed);
            printf("Thread %d (Worker: %s) concluiu check-in de passageiros.\n", id, args->name);
            break;
            
        case 4: // Portão
            // Esperar pela conclusão da segurança e bagagem
            pthread_mutex_lock(&mutex_status);
            while (status_steps[1] == 0 || status_steps[2] == 0) { // Aguarda bagagem (ID 1) e segurança (ID 2)
                printf("Thread %d (Worker: %s) aguardando conclusão de segurança e bagagem...\n", id, args->name);
                if (status_steps[1] == 0) {
                    pthread_cond_wait(&cond_baggage_completed, &mutex_status);
                }
                if (status_steps[2] == 0) {
                    pthread_cond_wait(&cond_security_completed, &mutex_status);
                }
            }
            pthread_mutex_unlock(&mutex_status);
            
            // Liberar o portão
            printf("Thread %d (Worker: %s) preparando o portão de embarque...\n", id, args->name);
            sleep(1); // Simulando o tempo de preparação do portão
            
            // Atualizar status do portão (seção crítica)
            pthread_mutex_lock(&mutex_gate);
            gate_open = 1;
            pthread_mutex_unlock(&mutex_gate);
            
            // Atualizar status da etapa
            pthread_mutex_lock(&mutex_status);
            status_steps[id] = 1;
            pthread_mutex_unlock(&mutex_status);
            
            // Sinalizar que o portão foi liberado
            pthread_cond_broadcast(&cond_gate_open);
            printf("Thread %d (Worker: %s) liberou o portão para embarque.\n", id, args->name);
            break;
    }
    
    // Log de finalização da thread
    printf("Thread %d (Worker: %s) finalizada.\n", id, args->name);
    pthread_exit(NULL);
}

// Função para imprimir o resumo da simulação
void print_simulation_summary(argsThreads args[]) {
    // Resumo da simulação
    printf("\nResumo da Simulação:\n");
    printf("Passageiros processados: %d\n", passengers_processed);
    printf("Bagagens verificadas: %d\n", checked_baggage);
    printf("Status do portão: %s\n", gate_open ? "Liberado" : "Fechado");
    printf("Status das etapas: ");
    for (int i = 0; i < MAX_THREADS; i++) {
        printf("%s: %s", args[i].name, status_steps[i] ? "Concluído" : "Pendente");
        if (i < MAX_THREADS - 1) {  // Adiciona " -> " apenas se não for o último item
            printf(" -> ");
        }
    }
    printf("\n");
    
    // Finaliza o programa com sucesso
    printf("\nSimulação concluída. Todas as etapas foram executadas com sucesso!\n");
}