#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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
    qtde_numeros = (*qtde_pontos) * (*dim_pontos); //calcula a qtde de elementos da matriz

    /*printf("qtde pontos = %ld\n", *qtde_pontos);
    printf("dim pontos = %d\n", *dim_pontos);
    printf("qtde numeros = %lld\n", qtde_numeros);*/

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

    fprintf(stdout, "Leitura dos Pontos:\n");
    //imprime a matriz na saida padrao
    for(i = 0; i < qtde_numeros; i += (*dim_pontos)) {
        for(j = i; j < i + (*dim_pontos); j++){
            fprintf(stdout, "%.6f ", pontos[j]);
        } fprintf(stdout, "\n");
    }

    //finaliza o uso das variaveis
    fclose(descritorArquivo);
    return pontos;
}

int main(int argc, char* argv[]){

    if(argc < 4){
        fprintf(stderr, "Digite: %s <arquivo entrada> <qtde clusters> <qtde threads>\n", argv[0]);
        return 1;
    }

    long int i;
    int j;

    int achou;
    char* arquivo = argv[1];
    float* pontos;
    float* clusters;
    long int qtde_pontos;
    int dim_pontos;
    long long int qtde_numeros;
    int qtde_clusters = atoi(argv[2]);
    int qtde_threads = atoi(argv[3]);

    pontos = lePontosBinario(arquivo, &qtde_pontos, &dim_pontos);
    if (pontos == NULL)
        return 2;
    
    qtde_numeros = qtde_pontos * dim_pontos;

    /*
    printf("qtde_pontos = %ld\n", qtde_pontos);
    printf("dim_pontos = %d\n", dim_pontos);
    printf("qtde_numeros = %lld\n", qtde_numeros);
    */

    // gerando os centróides
    srand(time(NULL));
    int cluster_ids[qtde_clusters]; // lista de índices dos centróides
    for(i = 0; i < qtde_clusters; i++){
        int i_ponto;
		do{
			achou = 0;
            i_ponto = rand() % qtde_pontos;
			for(j = 0; j <= i; j++){
				if(i_ponto == cluster_ids[j]){
					achou = 1;
					break;
				}
			}
		}while(achou);
		cluster_ids[i] = i_ponto;
	}

    /*
    for(i = 0; i < qtde_clusters; i++)
        printf("cl = %d\n", cluster_ids[i]);
    */

    for(i = 0; i < qtde_clusters; i++){
        for(j = 0; j < dim_pontos; j++)
            clusters[i * dim_pontos + j] = pontos[i * dim_pontos + j];
    }

    
}