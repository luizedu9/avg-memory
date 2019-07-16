/*
// * File:      avg-memory.c
// * Autor:     Alissom Vieira da Cunha, Luiz Eduardo Pereira
// * Matricula: 0021623, 0021619
// * Data:      23/06/2016
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

//Struct que guarda o conjunto XY;
struct conjunto {
    double x; //Tempo;
    double y; //Memória
};
typedef struct conjunto Conjunto;

//Struct que guarda todas as informações de um ponto. (Conjunto, MAX/MIN, Quantidade de Pontos);
struct ponto {
    Conjunto *conjunto; //Ponteiro para o vetor de pontos (X/Y);
    int n; //Número de pares ordenados;
    double xmax, xmin, ymax, ymin;
};
typedef struct ponto Ponto;

//Atribui valores a "Ponto" a partir de um arquivo de entrada;
Ponto CarregaArquivo(char *nome) {
    Ponto ponto;
    int i, cont = 0;
	float lixo;
    FILE *file;
    file = fopen(nome, "r"); /*Aberto em modo leitura*/
    while (fscanf(file, "%f%f", &lixo, &lixo) != EOF) { //Conta a quantidade de linhas;
       cont++;
    }
    fclose(file);
    file = fopen(nome, "r");
    ponto.n = 0;
    ponto.conjunto = (Conjunto*) malloc((cont + 1) * sizeof(Conjunto));
    for (i = 1; i <= cont; i++) {
        fscanf(file, "%lf%lf", &ponto.conjunto[i].x, &ponto.conjunto[i].y); 
        ponto.n++;
        if (i == 1) {
            ponto.xmax = ponto.conjunto[i].x;
            ponto.xmin = ponto.conjunto[i].x;
            ponto.ymax = ponto.conjunto[i].y;
        }
        if (ponto.conjunto[i].x > ponto.xmax)
            ponto.xmax = ponto.conjunto[i].x;
        if (ponto.conjunto[i].x < ponto.xmin)
            ponto.xmin = ponto.conjunto[i].x;
        if (ponto.conjunto[i].y > ponto.ymax)
            ponto.ymax = ponto.conjunto[i].y;       
    }
    ponto.ymax = (ponto.ymax * 0.2) + ponto.ymax; //Aumenta 20% da area em Y;
    ponto.ymin = 0;
    fclose(file);
    return(ponto);
}

//Calcula a Derivada Segunda(s2) de um conjunto de pares ordenados;
double* CalculaDerivadaSpline(Ponto ponto) {
    int i, m, n = ponto.n;
    double DeltaA, DeltaB, t, Ha, Hb;
    double d[n - 1], e[n - 1];
    double *s2;
    s2 = (double*) malloc(n * sizeof(double));
    
    if (n < 3) {
        printf("Erro - Pontos insuficientes"); //Em uma TAD de uso generico não é adequado imprimir nada na tela, mas como
        exit(0); //Interrompe execução;        //esse não é o objetivo do trabalho, fiz o tratamento de erro aqui mesmo.
    }
    
    //Sistema Tridiagora Simétrico:   
    m = n - 2;
    Ha = ponto.conjunto[2].x - ponto.conjunto[1].x;
    DeltaA = (ponto.conjunto[2].y - ponto.conjunto[1].y) / Ha;
    for (i = 1; i <= m; i++) {
        Hb = ponto.conjunto[i + 2].x - ponto.conjunto[i + 1].x;
        DeltaB = (ponto.conjunto[i + 2].y - ponto.conjunto[i + 1].y) / Hb;
        e[i] = Hb;
        d[i] = 2 * (Ha + Hb);
        s2[i + 1] = 6 * (DeltaB - DeltaA);
        Ha = Hb;
        DeltaA = DeltaB;
    }
    
    //Eliminaçao de Gauss:
    for (i = 2; i <= m; i++) {
        t = e[i - 1] / d[i - 1];
        d[i] = d[i] - t * e[i - 1];
        s2[i + 1] = s2[i + 1] - t * s2[i];
    }
    
    //Substituição Retroativa:
    s2[m + 1] = s2[m + 1] / d[m];
    for (i = m; i >= 2; i--)
        s2[i] = (s2[i] - e[i - 1] * s2[i + 1]) / d[i - 1];
    s2[1] = 0;
    s2[m + 2] = 0;

    return(s2);
}

//Interpolação - Encontra o valor de Y em determinado X;
double AvaliaSpline(Ponto ponto, double *s2, double valor) {
    int inf, sup, indice, n = ponto.n;
    double h, a, b, c, d;
    
    if ((valor < ponto.conjunto[1].x) || (valor > ponto.conjunto[n].x)) {
        printf("Erro - Fora do intervalo");
        exit(0);
    }
    
    //Busca Binaria:
    inf = 1;
    sup = n;    
    while(sup - inf > 1) {
        indice = (int)((inf + sup) / 2);
        if (ponto.conjunto[indice].x > valor)
            sup = indice;
        else 
            inf = indice;
    }
    
    //Avaliação de P(x) por Honner:
    h = ponto.conjunto[sup].x - ponto.conjunto[inf].x;
    a = (s2[sup] - s2[inf]) / (6 * h);
    b = s2[inf] * 0.5;
    c = ((ponto.conjunto[sup].y - ponto.conjunto[inf].y) / h) - (s2[sup] + 2 * s2[inf]) * h / 6;
    d = ponto.conjunto[inf].y;
    h = valor - ponto.conjunto[inf].x;
    valor = ((a * h + b) * h + c) * h + d;
    
    return(valor);
}

//Cria um número aleatorio real;
double NumeroAleatorioDouble(int xmax, int xmin) {
    double A, B;
    A = (rand() % (xmax - xmin)) + xmin; //Parte inteira;
    B = (rand() % 9999) / (double)10000; //Parte decimal; 
    return(A + B);
}

//Encontra a integral da função;
double IntegralMonteCarlo(Ponto ponto, double *s2, int n) {
    int abaixo, i, y;
    double x, aux, area, areatotal;
    abaixo = 0;
    for (i = 1; i <= n; i++) {
        x = NumeroAleatorioDouble(ponto.xmax, ponto.xmin); //Número real entre o intervalo X MAX/MIN;
        y = (rand() % ((int)(ponto.ymax - ponto.ymin))) + ponto.ymin; //Número inteiro entre o intervalo Y MAX/MIN;
        aux = AvaliaSpline(ponto, s2, x);
        if (y <= aux)
            abaixo++;
    }
    areatotal = (ponto.xmax - ponto.xmin) * (ponto.ymax - ponto.ymin);
    area = areatotal * ((double)abaixo / (double)n);
    return(area);
}

//Teorema do Valor Médio para Integrais;
double TVMI(double Integral, Ponto ponto) {
    double UsoMedio;
    UsoMedio = Integral / (ponto.xmax - ponto.xmin);
    return(UsoMedio);
}

//Cria o arquivo de saida em R e via terminal;
void SaidaDados(Ponto ponto, char *saida, double *s2) {
    int n, i = 0;
    double aux, Integral, UsoMedio;
    char saidaR[strlen(saida) + 2], aux1 = 'a';
    FILE *file;

    while (aux1 != '\0') { //Cria uma nova string para concatenar <arquivo saida> com ".r";
        saidaR[i] = saida[i];
        i++;
        aux1 = saida[i];
    }
    saidaR[i] = '\0';
    strcat(saidaR, ".r");
	
    file = fopen(saidaR, "wt");
    
    Integral = IntegralMonteCarlo(ponto, s2, 10000000); //Calcula integral;
    UsoMedio = TVMI(Integral, ponto); //Calcula uso medio;

    //Salva no Arquivo.r
    fprintf(file, "#\n# Generated automatically by \"avg-memory\" application\n#\n\n");
    fprintf(file, "# Original points (x coordinates)\n");   
    fprintf(file, "xorig <- c(");
    for (i = 1; i < ponto.n; i++) {
        fprintf(file, "\n%.0f", ponto.conjunto[i].x);
        if (i != (ponto.n - 1))
            fprintf(file, ",");
    }
    fprintf(file, "\n);\n\n");
    
    fprintf(file, "# Original points (y coordinates)\n");
    fprintf(file, "yorig <- c(");
    for (i = 1; i < ponto.n; i++) {
        fprintf(file, "\n%.0f", ponto.conjunto[i].y);
        if (i != (ponto.n - 1))
            fprintf(file, ",");
    }
    fprintf(file, "\n);\n\n");
    
    fprintf(file, "# Spline points (x coordinates, sampling interval = 0.01)\n");
    fprintf(file, "xspl <- c(");
    n = (ponto.n - 1) * 10; //(n - 1) para contar de 0 a (tamanho de x) e (* 10) para contar os decimais)
    for (i = 0; i < n; i++) {
        aux = (double)i / 10;
        fprintf(file, "\n%.2f", aux);
        if (i != (n - 1))
            fprintf(file, ",");
    }
    fprintf(file, "\n);\n\n");
    
    fprintf(file, "# Spline points (y coordinates, sampling interval = 0.01)\n");
    fprintf(file, "yspl <- c(");
    n = (ponto.n - 1) * 10; //(n - 1) para contar de 0 a (tamanho de x) e (* 10) para contar os decimais)
    for (i = 0; i < n; i++) {
        aux = (double)i / 10;
        aux = AvaliaSpline(ponto, s2, aux);
        fprintf(file, "\n%f", aux);
        if (i != (n - 1))
            fprintf(file, ",");
    }
    fprintf(file, "\n);\n\n");
    
    fprintf(file, "# Average Memory Usage\n");
    fprintf(file, "AvgMemory <- %f;\n\n", UsoMedio);
    fprintf(file, "# Plot the values in .png file\n");
    fprintf(file, "png(file=\"%s.png\", width=1200);\n", saida); //********   %s aqui
    fprintf(file, "title <- paste(\"AVG Memory Usage: %f Kb (%d Samples)\");", UsoMedio, ponto.n);
    fprintf(file, "plot(xspl, yspl, type=\"l\", col=\"blue\", main=title, xlab=\"Samples\", ylab=\"Mem. Usage\", lwd=3);\n");
    fprintf(file, "points(xorig, yorig, pch=19, col=\"red\");\n");
    fprintf(file, "lines( c(min(xorig), max(xorig)), c(AvgMemory, AvgMemory), col=\"black\", lty=2, lwd=3);\n");
    fprintf(file, "lixo <- dev.off();");
    
    fclose(file);
   
    //Imprime na Tela
    printf("Number of Samples   : %d\n", ponto.n);
    printf("Average Memory Usage: %f Kb\n", UsoMedio);
    printf("Run ‘Rscript %s’ to generate Average Memory Usage Chart", saidaR);
	
    return;
}

int main(int argc, char *argv[]) {
    double *s2;
    Ponto ponto;
    char *entrada, *saida;
    srand((unsigned)time(NULL)); //Seed para números aleatorios;	

    entrada = argv[1];
    saida = argv[2];
    ponto = CarregaArquivo(entrada);
    s2 = CalculaDerivadaSpline(ponto);
    SaidaDados(ponto, saida, s2);	

    free(ponto.conjunto);
    free(s2);
    
    return(0);
}
