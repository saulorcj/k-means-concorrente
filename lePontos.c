#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc, char* argv[]){

    if (argc < 2){
        fprintf(stderr, "Digite: %s <arquivo entrada>\n", argv[0]);
        return 1;
    }

    // para iteração
    long int i;
    int j;

    // variáveis
    long int qtde_pontos;
    int dim_pontos;
    long long int qtde_numeros;

    float* pontos;
    FILE* descritorArquivo;
    size_t ret;

    // abre o arquivo para leitura binaria
    descritorArquivo = fopen(argv[1], "rb");
    if(!descritorArquivo) {
       fprintf(stderr, "Erro de abertura do arquivo\n");
       return 2;
    }

    // le quantidade de pontos
    ret = fread(&qtde_pontos, sizeof(long int), 1, descritorArquivo);
    if(!ret) {
       fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo \n");
       return 3;
    }
    // le dimensão dos pontos
    ret = fread(&dim_pontos, sizeof(int), 1, descritorArquivo);
    if(!ret) {
       fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo \n");
       return 3;
    }
    qtde_numeros = qtde_pontos * dim_pontos; //calcula a qtde de elementos da matriz

    printf("qtde pontos = %ld\n", qtde_pontos);
    printf("dim pontos = %d\n", dim_pontos);
    printf("qtde numeros = %lld\n", qtde_numeros);

    //aloca espaço na memória
    pontos = (float*) malloc(sizeof(float) * qtde_numeros);
    if(!pontos) {
       fprintf(stderr, "Erro de alocao da memoria da matriz\n");
       return 3;
    }

    //carrega a matriz de elementos do tipo float do arquivo
    ret = fread(pontos, sizeof(float), qtde_numeros, descritorArquivo);
    if(ret < qtde_numeros) {
       fprintf(stderr, "Erro de leitura dos elementos da matriz\n");
       return 4;
    }

    //imprime a matriz na saida padrao
    for(i = 0; i < qtde_numeros; i += dim_pontos) {
        for(j = i; j < i + dim_pontos; j++){
            fprintf(stdout, "%.6f ", pontos[j]);
        } fprintf(stdout, "\n");
    }

    //finaliza o uso das variaveis
    fclose(descritorArquivo);
    free(pontos);
    return 0;
}