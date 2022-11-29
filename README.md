# k-means-concorrente

O arquivo main_.c contém a implementação concorrente do algoritmo k-means. Deve ser compilado usando -lpthread.
Ao executar o main_, devem ser passados como argumentos adicionais na linha de comando: <br/>
1.Nome do arquivo de entrada, 2. quantidade de clusters desejada, 3. quantidade de threads, 4. nome do arquivo de saída em binário (incluindo ".bin"), 5. nome do arquivo de saída em txt (incluindo o ".txt"). <br/>
O arquivo de entrada deve ser um arquivo binário com os pontos, gerado pelo executável obtido da compilação do arquivo geraPontos.c. Este arquivo gera um arquivo binário e um arquivo txt com os pontos.<br/>
Ao executar geraPontos, deve-se passar como argumentos adicionais na linha de comando: <br/>
1. Quantidade de pontos, 2. número de dimensões dos pontos, 3. limite para as coordenadas de cada ponto, 4. nome do arquivo de saída binário (incluindo ".bin"), 5. nome do arquivo de saída txt (incluindo ".txt") <br/>
<br/>
Para os testes de corretude, os arquivos das pastas "Entradas" e "Saidas" deste repositório devem estar todos na mesma pasta do notebook python para que a execução ocorra corretamente.
