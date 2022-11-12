#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc, char *argv[]){

    // confere quantidade de argumentos
    if (argc < 5){
        fprintf(stderr, "Digite: %s <qtde pontos> <dim pontos> <limite> <arquivo saida>\n", argv[0]);
        return 1;
    }

    // variáveis
    long int qtde_pontos = atoi(argv[1]); // quantidade de pontos
    int dim_pontos = atoi(argv[2]); // dimensão dos pontos
    int limite = atoi(argv[3]); // valor máximo para uma coordenada
    long long int qtde_numeros = qtde_pontos * dim_pontos; // quantidade de números

    float* pontos;
    FILE* descritorArquivo; //descritor do arquivo de saida
    size_t ret; //retorno da funcao de escrita no arquivo de saida

    /* DEBUG
    printf("qtde_pontos = %ld\n", qtde_pontos);
    printf("dim_pontos = %d\n", dim_pontos);
    printf("qtde_numeros = %lld\n", qtde_numeros);
    */

    // alocando espaço na memória
    pontos = (float*) malloc(sizeof(float) * qtde_numeros);
    if (!pontos){
        fprintf(stderr, "Erro de alocao da memoria da matriz\n");
        return 2;
    }

    // preenchendo os pontos
    srand(time(NULL));
    for(long int i = 0; i < qtde_numeros; i++)
        *(pontos + i) = (rand() % limite);
    
    // abrindo o arquivo
    descritorArquivo = fopen(argv[4], "wb");
    if(!descritorArquivo) {
       fprintf(stderr, "Erro de abertura do arquivo\n");
       return 3;
    }

    // escreve quantidade de pontos
    ret = fwrite(&qtde_pontos, sizeof(long int), 1, descritorArquivo);
    // escreve dimensão dos pontos
    ret = fwrite(&dim_pontos, sizeof(int), 1, descritorArquivo);
    // escreve os pontos
    ret = fwrite(pontos, sizeof(float), qtde_numeros, descritorArquivo);
    if(ret < qtde_numeros) {
       fprintf(stderr, "Erro de escrita no  arquivo\n");
       return 4;
    }

    //finaliza o uso das variaveis
    fclose(descritorArquivo);
    free(pontos);
    return 0;
}