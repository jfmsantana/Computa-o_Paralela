#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define LIMIAR 200 // Valor mínimo de brilho para considerar pixel como estrela

/* ============================================================
   Função: lerPGM
   Lê uma imagem PGM no formato P2 (ASCII) e retorna um vetor
   linear com os pixels. Largura e altura são retornados por referência.
   ============================================================ */
int *lerPGM(const char *filename, int *largura, int *altura)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        perror("Erro abrindo imagem");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    char formato[3];
    fscanf(f, "%2s", formato);
    if (formato[0] != 'P' || formato[1] != '2')
    {
        printf("Formato não suportado (use P2 ASCII)\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int maxval;
    fscanf(f, "%d %d %d", largura, altura, &maxval);

    int tamanho = (*largura) * (*altura);
    int *dados = (int *)malloc(sizeof(int) * tamanho);
    for (int i = 0; i < tamanho; i++)
    {
        fscanf(f, "%d", &dados[i]);
    }

    fclose(f);
    return dados;
}

/* ============================================================
   Função: contaEstrelas
   Recebe um bloco de pixels e conta quantas estrelas existem
   usando um flood-fill (BFS) para agrupar pixels vizinhos brilhantes.
   ============================================================ */
int contaEstrelas(int *img, int largura, int altura)
{
    int *visitado = (int *)calloc(largura * altura, sizeof(int));
    int estrelas = 0;

    // deslocamentos para vizinhança de 8 direções
    int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int y = 0; y < altura; y++)
    {
        for (int x = 0; x < largura; x++)
        {
            int idx = y * largura + x;

            // encontrou pixel brilhante não visitado → nova estrela
            if (img[idx] > LIMIAR && !visitado[idx])
            {
                estrelas++;

                // fila para explorar todos os pixels conectados (BFS)
                int *fila = (int *)malloc(sizeof(int) * largura * altura);
                int frente = 0, tras = 0;
                fila[tras++] = idx;
                visitado[idx] = 1;

                while (frente < tras)
                {
                    int atual = fila[frente++];
                    int ay = atual / largura;
                    int ax = atual % largura;

                    // varre vizinhos 8-direções
                    for (int k = 0; k < 8; k++)
                    {
                        int nx = ax + dx[k];
                        int ny = ay + dy[k];
                        if (nx >= 0 && nx < largura && ny >= 0 && ny < altura)
                        {
                            int nidx = ny * largura + nx;
                            if (img[nidx] > LIMIAR && !visitado[nidx])
                            {
                                visitado[nidx] = 1;
                                fila[tras++] = nidx;
                            }
                        }
                    }
                }
                free(fila);
            }
        }
    }

    free(visitado);
    return estrelas;
}

/* ============================================================
   Função principal:
   Modelo mestre/escravo com MPI para contar estrelas.
   O mestre lê a imagem, divide em blocos e envia aos escravos.
   Cada escravo conta as estrelas do seu bloco e devolve ao mestre.
   ============================================================ */
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int largura, altura;
    int *imagem = NULL;

    // ===== Mestre =====
    if (rank == 0)
    {
        if (argc < 2)
        {
            printf("Uso: mpirun -np N ./star_counter imagem.pgm\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        imagem = lerPGM(argv[1], &largura, &altura);
        printf("Imagem %dx%d carregada.\n", largura, altura);
    }

    // Envia dimensões para todos
    MPI_Bcast(&largura, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&altura, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int processos_escravos = size - 1;
    if (processos_escravos <= 0)
    {
        if (rank == 0)
            printf("Precisa de pelo menos 2 processos.\n");
        MPI_Finalize();
        return 0;
    }

    int linhas_por_proc = altura / processos_escravos;

    // ===== Mestre envia blocos =====
    if (rank == 0)
    {
        for (int dest = 1; dest <= processos_escravos; dest++)
        {
            int inicio = (dest - 1) * linhas_por_proc;
            int fim = (dest == processos_escravos) ? altura : inicio + linhas_por_proc;

            // adiciona bordas para evitar cortes de estrelas entre processos
            int inicio_envio = (inicio == 0) ? inicio : inicio - 1; // borda acima
            int fim_envio = (fim == altura) ? fim : fim + 1;        // borda abaixo
            int linhas_envio = fim_envio - inicio_envio;

            MPI_Send(&linhas_envio, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            MPI_Send(imagem + inicio_envio * largura,
                     linhas_envio * largura, MPI_INT,
                     dest, 0, MPI_COMM_WORLD);
        }

        // recebe parciais e soma
        int totalEstrelas = 0;
        for (int src = 1; src <= processos_escravos; src++)
        {
            int parcial;
            MPI_Recv(&parcial, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            totalEstrelas += parcial;
        }

        printf("Total de estrelas detectadas: %d\n", totalEstrelas);
        free(imagem);
    }

    // ===== Escravos =====
    else
    {
        int linhas_recebidas;
        MPI_Recv(&linhas_recebidas, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *bloco = (int *)malloc(sizeof(int) * linhas_recebidas * largura);
        MPI_Recv(bloco, linhas_recebidas * largura, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // ajusta para ignorar linhas de borda artificiais
        int linhas_validas = linhas_recebidas;
        int offset = 0;
        if (rank != 1)
        {                     // não é o primeiro processo
            offset = largura; // pula linha fantasma superior
            linhas_validas--;
        }
        if (rank != size - 1)
        { // não é o último processo
            linhas_validas--;
        }

        int estrelas = contaEstrelas(bloco + offset, largura, linhas_validas);
        MPI_Send(&estrelas, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        free(bloco);
    }

    MPI_Finalize();
    return 0;
}
