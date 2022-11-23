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

/*
acabou é usada como bool, determina se terminou a clusterização
Se não houve mudança entre os centróides, então acabou vai pra 1
na thread 0 e todas as threads, ao ver isto, dão break;
calloc_feito é pra saber se o ponteiro qtde_pontos_centroide
já foi todo para zero por alguma thread
*/
int bloqueadas = 0, acabou = 0, calloc_feito = 0;

pthread_mutex_t x_mutex, y_mutex; //x é o da barreira, y é o da exclusão mútua pro qtde_ponts_centroide
pthread_cond_t x_cond;
int* qtde_pontos_centroide;
float* new_centroides; //cada tread vai colocar as somas e depois dividir nesse vetor

//Barreira
void barreira(int nthreads) {
    pthread_mutex_lock(&x_mutex); //inicio secao critica
    if (bloqueadas == (nthreads - 1)) { 
      //ultima thread a chegar na barreira
      pthread_cond_broadcast(&x_cond);
      bloqueadas=0;
    } else {
      bloqueadas++;
      pthread_cond_wait(&x_cond, &x_mutex);
    }
    pthread_mutex_unlock(&x_mutex); //fim secao critica
}



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
    // id da thread
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
    int centroide_proximo;
    //pra saber se achou centroide diferente no final
    int achou;

    while(1){

        if(acabou)
            break;

        /*RELACIONA CADA PONTO AO SEU CENTRÓIDE MAIS PRÓXIMO*/

        for(i = id; i < qtde_pontos; i+= qtde_threads){
            menor_centroide = centroide_mais_proximo(i * dimensao, dimensao, qtde_centroides, centroides_anterior, pontos);
            //printf("ponto[%ld] = centroide[%d]\n", i, menor_centroide);
            pontos_centroides[i] = menor_centroide;
        }

        /*BARREIRA*/
        barreira(qtde_threads);


        pthread_mutex_lock(&y_mutex);
        if(calloc_feito) //não precisa fazer nada
            pthread_mutex_unlock(&y_mutex);    
        else{
            qtde_pontos_centroide = (int*) calloc(qtde_centroides, sizeof(int));//é global, por isso o cuidado
            calloc_feito = 1;
            pthread_mutex_unlock(&y_mutex);
        }
        /*CALCULA SOMA E QUANTIDADE DOS PONTOS PARA CADA CLUSTER*/
        // gerando a soma e quantidade de pontos por centróide
        
        centroides_posterior = (float*) calloc(qtde_centroides * dimensao, sizeof(float));

        for(i = id; i < qtde_pontos; i+= qtde_threads){
            centroide_proximo = pontos_centroides[i];
            pthread_mutex_lock(&y_mutex);
            qtde_pontos_centroide[centroide_proximo]++;
            pthread_mutex_unlock(&y_mutex);

            for(long long int j = centroide_proximo * dimensao, k = i * dimensao; j < (centroide_proximo + 1) * dimensao, k < (i + 1) * dimensao; j++, k++){
                centroides_posterior[j] += pontos[k];
            }

            //pthread_mutex_unlock(&y_mutex);
        }

        //soma no vetor global:
        pthread_mutex_lock(&y_mutex);
        for(i=0;i<qtde_centroides*dimensao;i++){
            new_centroides[i] += centroides_posterior[i];
        }
        pthread_mutex_unlock(&y_mutex);


        /*BARREIRA*/
        barreira(qtde_threads);  
        calloc_feito = 0; //reseta pra próxima iteração
        //até tem condição de corrida, mas todas estão setando pra 0 então não tem problema


        /*T1 CALCULA OS CENTRÓIDES E REGISTRA SE HOUVE MUDANÇA*/
        // dividindo a soma pela quantidade de pontos

        if(id==0){
            achou = 0; // registra se encontrou valores diferentes
            for(i = 0; i < qtde_centroides; i++){
                for(int j = i * dimensao; j < (i + 1) * dimensao; j++){
                    new_centroides[j] = new_centroides[j] / qtde_pontos_centroide[i];

                    if(fabs(new_centroides[j] - centroides_anterior[j]) > 1e-10)
                        achou = 1;
                }
            }

            free(qtde_pontos_centroide);

            if (!achou)
                acabou =1;
            
            if (!acabou){
                for(int k = 0; k < qtde_centroides * dimensao; k++){
                    centroides_anterior[k] = new_centroides[k];//pra próxima iteração 
                    new_centroides[k] = 0;
                }
            }
        }
        
        /*BARREIRA*/
        barreira(qtde_threads);
        //os que estou dando free não serão mais usados neste loop, e serão realocados depois
        free(centroides_posterior);
        /*CASO OS CENTRÓIDES NÃO TENHAM MUDADO, LOOP É INTERROMPIDO*/
        //mas verifico no começo do loop
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

    // validando entrada
    if(argc < 5){
        fprintf(stderr, "Digite: %s <arquivo entrada> <qtde clusters> <qtde threads> <arquivo saida>\n", argv[0]);
        return 1;
    }
    // VARIÁVEIS DE ITERAÇÃO
    int i, j;

    //VARIÁVEIS DA BARREIRA
    pthread_mutex_init(&x_mutex, NULL);
    pthread_mutex_init(&y_mutex, NULL);
    pthread_cond_init (&x_cond, NULL);

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
    long int qtde_pontos;
    // dimensão dos pontos
    int dimensao;

    // threads
    pthread_t thread_id[qtde_threads];
    // argumentos de entrada para as threads
    t_Args *args;

    FILE* descritor;
    size_t ret;

    descritor = fopen(argv[4], "wb");
    if (!descritor){
        fprintf(stderr, "Erro de abertura do arquivo de saída\n");
        return 2;
    }

    pontos = lePontosBinario(arquivo, &qtde_pontos, &dimensao);
    if (pontos == NULL)
        return 2;

    pontos_centroides = malloc(sizeof(int) * qtde_pontos);
    new_centroides = (float*) calloc(qtde_centroides*dimensao, sizeof(float));//global;compartilhado pelas threads

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

    // escreve quantidade de pontos
    ret = fwrite(&qtde_pontos, sizeof(long int), 1, descritor);
    // escreve quantidade de centróides
    ret = fwrite(&qtde_centroides, sizeof(int), 1, descritor);
    // escreve dimensão
    ret = fwrite(&dimensao, sizeof(int), 1, descritor);
    // escreve os centróides
    ret = fwrite(centroides, sizeof(float), qtde_centroides * dimensao, descritor);
    // escreve o índice dos centróides por ponto
    ret = fwrite(pontos_centroides, sizeof(int), qtde_pontos, descritor);

    /*DEBUG*/
    /*
    printf("centroides\n");
    for(int k = 0; k < qtde_centroides; k++){
        printf("centroide[%d] = ", k);
        for(int j = k * dimensao; j < (k + 1) * dimensao; j++){
            printf("%.6f ", new_centroides[j]);
        } printf("\n");
    }
    printf("pontos_centroides\n");
    for(i = 0; i < qtde_pontos; i++){
        //menor_centroide = centroide_mais_proximo(i * dimensao, dimensao, qtde_centroides, centroides_anterior, pontos);
        printf("ponto[%d] = centroide[%d]\n", i, pontos_centroides[i]);
        //pontos_centroides[i] = menor_centroide;
    }*/

    /*ESCRITA NO ARQUIVO*/
    free(pontos);
    free(centroides);
    free(pontos_centroides);
    return 0;
}
