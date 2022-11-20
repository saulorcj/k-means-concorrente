#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#ifndef _CLOCK_TIMER_H
#define _CLOCK_TIMER_H

#include <sys/time.h>
#define BILLION 1000000000L

#define GET_TIME(now) { \
   struct timespec time; \
   clock_gettime(CLOCK_MONOTONIC, &time); \
   now = time.tv_sec + time.tv_nsec/1000000000.0; \
}
#endif

float* lePontosBinario(char *arquivo, long int* qtde_pontos, int* dim_pontos){
    // para iteração
    long int i;
    int j;

    // variáveis
    float* pontos;
    long long int qtde_numeros;
    FILE* descritorArquivo;
    size_t ret;

    // abre o arquivo para leitura binaria
    descritorArquivo = fopen(arquivo, "rb");
    if(!descritorArquivo) {
       fprintf(stderr, "Erro de abertura do arquivo\n");
       return NULL;
    }

    // le quantidade de pontos
    ret = fread(qtde_pontos, sizeof(long int), 1, descritorArquivo);
    if(!ret) {
       fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo \n");
       return NULL;
    }

    // le dimensão dos pontos
    ret = fread(dim_pontos, sizeof(int), 1, descritorArquivo);
    if(!ret) {
       fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo \n");
       return NULL;
    }

    // quantidade de números
    qtde_numeros = (*qtde_pontos) * (*dim_pontos); //calcula a qtde de elementos da matriz

    //aloca espaço na memória
    pontos = (float*) malloc(sizeof(float) * qtde_numeros);
    if(!pontos) {
       fprintf(stderr, "Erro de alocao da memoria da matriz\n");
       return NULL;
    }

    //carrega a matriz de elementos do tipo float do arquivo
    ret = fread(pontos, sizeof(float), qtde_numeros, descritorArquivo);
    if(ret < qtde_numeros) {
       fprintf(stderr, "Erro de leitura dos elementos da matriz\n");
       return NULL;
    }

    // imprime os pontos
    fprintf(stdout, "Pontos:\n");
    for(i = 0; i < qtde_numeros; i += (*dim_pontos)) {
        for(j = i; j < i + (*dim_pontos); j++){
            fprintf(stdout, "%.6f ", pontos[j]);
        } fprintf(stdout, "\n");
    }

    //finaliza o uso das variaveis
    fclose(descritorArquivo);
    return pontos;
}

int centroide_mais_proximo(int pont_comeco, int dimensao, int qtde_centroides, float* centroides, float* pontos){
    // iteração
    int j, k, l;
    // menor soma dos pontos
    double menor_somatorio = INFINITY;
    // índice do menor centróide
    int menor_centroide;
    // soma dos pontos
    float somatorio;
    // índice + 1 da última coordenada do ponto
    int pont_fim = pont_comeco + dimensao;
    // índice da primeira coordenada do centróide
    int cent_comeco;
    // índice + 1 da última coordenada do centróide
    int cent_fim;

    for(j = 0; j < qtde_centroides; j++){
        somatorio = 0;
        cent_comeco = j * dimensao;
        cent_fim = cent_comeco + dimensao;

        // soma das diferenças de cada eixo
        for(k = pont_comeco, l = cent_comeco; k < pont_fim, l < cent_fim; k++, l++)
            somatorio += pow(pontos[k] - centroides[l], 2.0);

        // raiz da soma, a distância
        somatorio = pow(somatorio, 1.0/2.0);

        // verifica qual distância é a menor
        if (somatorio < menor_somatorio){
            menor_somatorio = somatorio;
            menor_centroide = j;
        }
    }

    return menor_centroide;
}

typedef struct {
    int id, dimensao, qtde_pontos, qtde_centroides, qtde_threads;
    int *pontos_centroides;
    float *pontos, *centroides;
} t_Args;

void *T(void* arg){
    t_Args* args = (t_Args*) arg;

    // VARIÁVEIS DE ITERAÇÂO
    // para iterar a quantidade de pontos
    long int i;

    // VARIÁVEIS PRINCIPAIS
    // id ad thread
    int id = args->id;
    // dimensão dos pontos
    int dimensao = args->dimensao;
    // quantidade de pontos
    long int qtde_pontos = args->qtde_pontos;
    // quantidade de centróides
    int qtde_centroides = args->qtde_centroides;
    // quantidade de threads
    int qtde_threads = args->qtde_threads;
    // pontos
    float* pontos = args->pontos;
    // centróides anteriores
    float* centroides_anterior = args->centroides;
    // centróides novos
    float* centroides_posterior;
    // índice do centróide para cada ponto
    int* pontos_centroides = args->pontos_centroides;

    // VARIÁVEIS TEMPORÁRIAS
    // índice do menor centróide
    int menor_centroide;

    while(1){
        /*RELACIONA CADA PONTO AO SEU CENTRÓIDE MAIS PRÓXIMO*/

        pontos_centroides = malloc(sizeof(int) * qtde_pontos);
        for(i = 0; i < qtde_pontos; i++){
            menor_centroide = centroide_mais_proximo(i * dimensao, dimensao, qtde_centroides, centroides_anterior, pontos);
            //printf("ponto[%ld] = centroide[%d]\n", i, menor_centroide);
            pontos_centroides[i] = menor_centroide;
        }

        /*BARREIRA*/
        /*CALCULA SOMA E QUANTIDADE DOS PONTOS PARA CADA CLUSTER*/
        /*BARREIRA*/
        /*T1 CALCULA OS CENTRÓIDES E REGISTRA SE HOUVE MUDANÇA*/
        /*BARREIRA*/
        /*CASO OS CENTRÓIDES NÃO TENHAM MUDADO, LOOP É INTERROMPIDO*/
        break;
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

    // validando entrada
    if(argc < 4){
        fprintf(stderr, "Digite: %s <arquivo entrada> <qtde clusters> <qtde threads>\n", argv[0]);
        return 1;
    }
    // VARIÁVEIS DE ITERAÇÃO
    int i;

    // VARIÁVEIS PRINCIPAIS
    // nome do arquivo
    char* arquivo = argv[1];
    // pontos
    float* pontos;
    // centróides
    float* centroides;
    // índice dos centróides para cada ponto
    int* pontos_centroides;
    // quantidade de centroides
    int qtde_centroides = atoi(argv[2]);
    // quantidade de threads
    int qtde_threads = atoi(argv[3]);
    // quantidade de pontos
    int qtde_pontos;
    // dimensão dos pontos
    int dimensao;

    // threads
    pthread_t thread_id[qtde_threads];
    // argumentos de entrada para as threads
    t_Args *args;

    pontos = lePontosBinario(arquivo, &qtde_pontos, &dimensao);
    if (pontos == NULL)
        return 2;

    centroides = malloc(sizeof(float) * qtde_centroides * dimensao);
    // usando os primeiros pontos como centróides
    for(i = 0; i < qtde_centroides * dimensao; i++)
        centroides[i] = pontos[i];
    
    args = malloc(sizeof(t_Args) * qtde_threads);
    for(i = 0; i < qtde_threads; i++){
        (args + i)->id = i;
        (args + i)->dimensao = dimensao;
        (args + i)->qtde_pontos = qtde_pontos;
        (args + i)->qtde_centroides = qtde_centroides;
        (args + i)->qtde_threads = qtde_threads;
        (args + i)->pontos = pontos;
        (args + i)->centroides = centroides;
        (args + i)->pontos_centroides = pontos_centroides;

        if(pthread_create(&thread_id[i], NULL, T, (void*) (args + i))){
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    for(i = 0; i < qtde_threads; i++){
        if (pthread_join(thread_id[i], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1); 
        }
    }

    /*ESCRITA NO ARQUIVO*/

    free(pontos);
    free(arquivo);
    free(centroides);
    free(pontos_centroides);
    return 0;
}