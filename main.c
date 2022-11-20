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

int centroide_mais_proximo(int i_comeco, int dim_pontos, int qtde_centroides, float* centroides, float* pontos){
    double menor_somatorio = INFINITY;
    int menor_centroide = 0;
    for(int j = 0; j < qtde_centroides; j++){
        float somatorio = 0;
        for(int k = i_comeco, l = j * dim_pontos; k < i_comeco + dim_pontos, l < (j + 1) * dim_pontos; k++, l++){
            somatorio += pow(pontos[k] - centroides[l], 2);
        }
        somatorio = pow(somatorio, 1.0/2.0);
        if (somatorio < menor_somatorio){
            menor_somatorio = somatorio;
            menor_centroide = j;
        }
    }

    return menor_centroide;
}

int main(int argc, char* argv[]){

    // validando entrada
    if(argc < 4){
        fprintf(stderr, "Digite: %s <arquivo entrada> <qtde clusters> <qtde threads>\n", argv[0]);
        return 1;
    }

    // iteração
    long int i;
    int j, k;

    // nome do arquivo que contém os pontos
    char* arquivo = argv[1];
    // array dos pontos
    float* pontos;
    int qtde_clusters = atoi(argv[2]);
    // quantidade de threads
    int qtde_threads = atoi(argv[3]);

    long int qtde_pontos;
    int dim_pontos;
    long long int qtde_numeros;
    // quantidade de clusters

    pontos = lePontosBinario(arquivo, &qtde_pontos, &dim_pontos);
    if (pontos == NULL)
        return 2;

    qtde_numeros = qtde_pontos * dim_pontos;

    srand(time(NULL));
    // registra se índice é repetido
    int achou;
    // lista de índices dos centróides
    int centers_indices[qtde_clusters];
    for(j = 0; j < qtde_clusters; j++){
        // índice do ponto
        int indice;
		do{
			achou = 0;
            indice = rand() % qtde_pontos; // gera um índice aleatório
			for(k = 0; k <= j; k++){ // verifica se índice existe
				if(indice == centers_indices[k]){ // índice existe
					achou = 1;
					break;
				}
			}
		}while(achou); // caso índice já exista, gera um novo
		centers_indices[j] = indice; // adiciona o novo íncide no array
	}

    // array de centroides
    float* centers = malloc(sizeof(float) * qtde_clusters * dim_pontos);
    // preenche o array de centróides de acordo com os seus índices
    for(j = 0; j < qtde_clusters; j++){
        int indice = centers_indices[j]; // índice do array de índices de pontos
        int indice_comeco = indice * dim_pontos; // ínndice de ínicio

        printf("centroide[%d] = ", j);
        for(k = 0, i = indice_comeco; i < indice_comeco + dim_pontos; k++, i++){
            centers[j * dim_pontos + k] = pontos[i];
            printf("%.6f ", centers[j * dim_pontos + k]);
        } printf("\n");
    }

    // array onde cada inteiro representa o centróide mais próximo do ponto de mesma posição
    int* pontos_centroides;
    while(1){
        /*ASSOCIAR CADA PONTO AO SEU CENTRÓIDE MAIS PRÓXIMO*/

        pontos_centroides = malloc(sizeof(int) * qtde_pontos);
        for(i = 0; i < qtde_pontos; i++){
            int menor_centroide = centroide_mais_proximo(i * dim_pontos, dim_pontos, qtde_clusters, centers, pontos);
            printf("ponto[%ld] = centroide[%d]\n", i, menor_centroide);
            pontos_centroides[i] = menor_centroide;
        }


        /*CALCULAR OS NOVOS CENTRÓIDES*/

        // array dos novos centróides
        float* new_centers = (float*) calloc(qtde_clusters * dim_pontos, sizeof(float));
        // array de quantidade de pontos por centróide
        int* qtde_pontos_centroide = (int*) calloc(qtde_clusters, sizeof(int));

        int centroide_proximo;
        // gerando a soma e quantidade de pontos por centróide
        for(i = 0; i < qtde_pontos; i++){
            centroide_proximo = pontos_centroides[i];
            qtde_pontos_centroide[centroide_proximo]++;
            for(j = centroide_proximo * dim_pontos, k = i * dim_pontos; j < (centroide_proximo + 1) * dim_pontos, k < (i + 1) * dim_pontos; j++, k++){
                new_centers[j] += pontos[k];
            }
        }
        // dividindo a soma pela quantidade de pontos
        for(i = 0; i < qtde_clusters; i++){
            for(j = i * dim_pontos; j < (i + 1) * dim_pontos; j++)
                new_centers[j] = new_centers[j] / qtde_pontos_centroide[i];
        }

        /*VERIFICAR SE OS CENTRÓIDES MUDARAM*/

        achou = 0; // registra se encontrou valores diferentes
        for(i = 0; i < qtde_clusters; i++){
            if(fabs(new_centers[i] - centers[i]) > 1e-10){
                achou = 1;
                break;
            }
        }

        if (!achou){
            break;
        }
        else {
            centers = new_centers;
        }
        printf("etapa\n");
    }  

    free(pontos);
    free(centers);
    return 0;
}