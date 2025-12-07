#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h> // Para funções Windows (Clipboard, caminhos)
#include <time.h>    // Para srand()/rand()
#include <ctype.h>   // Para isspace(), isxdigit()
#include <stdint.h>  // Para tipos inteiros fixos

// ============================================================
// Macros
// ============================================================
#define MAX_1 2048       // Tamanho máximo para texto normal
#define MAX_2 4096       // Tamanho máximo para texto criptografado (em hex, o dobro)
#define MAX_PASSWORD 250 // Tamanho máximo da senha

// ============================================================
// Protótipos das funções
// ============================================================

void valida_entrada(char *str, size_t m);                                        // Valida entradas do usuário
void inputs(char *entrada, char *senha);                                         // Recebe entrada para criptografia
void inputs_hex(char *entrada, char *senha);                                     // Recebe entrada para descriptografia
void RandomPassword(char *senha);                                                // Gera senha aleatória
void criptografar(const char *texto, const char *senha, char *saida);            // Criptografa texto
void descriptografar(const char *criptografado, const char *senha, char *saida); // Descriptografa
void Copy_to_clipboard(const char *str);                                         // Copia para área de transferência
void Salva_txt(const char *saida);                                               // Salva em arquivo .txt

// ============================================================
// Funções auxiliares (static = visíveis apenas neste arquivo)
// ============================================================

// Limpa o buffer de entrada (stdin) até encontrar nova linha ou EOF(end of file)
static void clear_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; // Remove caracteres restantes do buffer
}

// Remove espaços em branco do início e fim de uma string
static void trim_inplace(char *s)
{
    if (!s)
        return; // Verifica se a string não é NULL

    // Remove espaços do início
    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;

    // Move a string para o início se necessário
    if (start != s)
        memmove(s, start, strlen(start) + 1);

    // Remove espaços do final
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
    {
        s[len - 1] = '\0';
        len--;
    }
}

// Função base para receber entradas (usada por inputs() e inputs_hex())
static void inputs_base(char *entrada, size_t entrada_max, char *senha, size_t senha_max, int modo_hex)
{
    // Recebe a senha
    printf("Digite a senha de 1-%lu caracteres: ", (unsigned long)(senha_max - 1));
    if (!fgets(senha, (int)senha_max, stdin))
        senha[0] = '\0'; // Em caso de erro

    senha[strcspn(senha, "\n")] = '\0'; // Remove \n do final
    trim_inplace(senha);                // Remove espaços extras
    valida_entrada(senha, senha_max);   // Valida o tamanho

    // Recebe o texto (modo diferente para criptografia/descriptografia)
    if (!modo_hex)
        printf("Digite o texto de 1-%lu caracteres: ", (unsigned long)(entrada_max - 1));
    else
        printf("Cole o texto criptografado (até %lu chars): ", (unsigned long)(entrada_max - 1));

    if (!fgets(entrada, (int)entrada_max, stdin))
        entrada[0] = '\0';

    entrada[strcspn(entrada, "\n")] = '\0';
    trim_inplace(entrada);
}

// Função pública para receber entradas normais (p/ criptografia)
void inputs(char *entrada, char *senha)
{
    inputs_base(entrada, MAX_1, senha, MAX_PASSWORD, 0);
}

// Função pública para receber entradas hex (p/ descriptografia)
void inputs_hex(char *entrada, char *senha)
{
    inputs_base(entrada, MAX_2, senha, MAX_PASSWORD, 1);
}

// ============================================================
// Main
// ============================================================
int main()
{
    srand((unsigned)time(NULL)); // Inicializa gerador de números aleatórios

    // Buffers para armazenar dados
    char senha[MAX_PASSWORD]; // Armazena senha
    char entrada[MAX_1];      // Armazena texto de entrada
    char saida[MAX_2];        // Armazena texto de saída

    int op = 0;     // Opção do menu
    int fechar = 0; // Flag para sair do programa

    do
    {
        system("cls"); // Limpa tela

        // Interface do menu com cores (códigos ANSI)
        printf("\033[1;36m====== SISTEMA DE CRIPTOGRAFIA ======\033[0m\n\n");
        printf("\033[1;33m-------- MENU --------\033[0m\n\n");
        printf("\033[1;32m1\033[0m. Criptografar um texto\n");
        printf("\033[1;32m2\033[0m. Descriptografar um texto\n");
        printf("\033[1;32m3\033[0m. Gerar uma senha\n");
        printf("\033[1;32m4\033[0m. Sair do programa\n\n");

        printf("Escolha uma opcao: ");
        if (scanf("%d", &op) != 1) // Lê opção
        {
            printf("Entrada invalida!\n");
            clear_stdin();
            Sleep(1000); // Pausa de 1 segundo
            continue;
        }
        clear_stdin();

        switch (op)
        {
        case 1: // Criptografar
            inputs(entrada, senha);
            criptografar(entrada, senha, saida);
            Salva_txt(saida);
            break;

        case 2: // Descriptografar
            inputs_hex(entrada, senha);
            descriptografar(entrada, senha, saida);
            Salva_txt(saida);
            break;

        case 3: // Gerar senha aleatória
            RandomPassword(senha);
            break;

        case 4: // Sair
            fechar = 1;
            printf("Saindo...\n");
            break;

        default:
            printf("Opcao invalida!\n");
            break;
        }

        // Pausa antes de voltar ao menu (exceto na saída)
        if (!fechar)
        {
            printf("\nPressione ENTER para voltar ao menu...");
            clear_stdin();
        }

    } while (!fechar); // Loop até usuário escolher sair

    return 0;
}

// ============================================================
// Função de criptografia (XOR + conversão para Hexadecimal)
// ============================================================
void criptografar(const char *texto, const char *senha, char *saida)
{
    size_t tam_txt = strlen(texto);
    size_t tam_psw = strlen(senha);
    size_t k = 0; // Índice para saída

    // Validações
    if (tam_psw == 0)
    {
        printf("Erro: senha vazia.\n");
        saida[0] = '\0';
        return;
    }

    if (tam_txt > MAX_1)
    {
        printf("Erro: texto muito longo (max %d caracteres).\n", MAX_1);
        saida[0] = '\0';
        return;
    }

    // Processa cada caractere do texto
    for (size_t i = 0; i < tam_txt; i++)
    {
        unsigned char x = (unsigned char)texto[i] ^ (unsigned char)senha[i % tam_psw];

        // Verifica espaço no buffer — precisa de 2 chars + terminador
        if (k + 2 >= MAX_2)
        {
            printf("Erro: buffer de saida insuficiente.\n");
            break;
        }

        // Converte byte para hexadecimal (2 caracteres)
        snprintf(&saida[k], MAX_2 - k, "%02X", x);
        k += 2;
    }

    // Garante terminação
    if (k < MAX_2)
        saida[k] = '\0';
    else
        saida[MAX_2 - 1] = '\0';
}

// ============================================================
// Função de descriptografia (Hex para binário + XOR)
// ============================================================
void descriptografar(const char *criptografado, const char *senha, char *saida)
{
    size_t tam_cripto = strlen(criptografado);
    size_t tam_psw = strlen(senha);
    size_t k = 0; // Índice para saída

    // Validações
    if (tam_psw == 0)
    {
        printf("Erro: senha vazia.\n");
        saida[0] = '\0';
        return;
    }

    if (tam_cripto % 2 != 0) // Hex deve ter número par de caracteres
    {
        printf("Erro: Texto enviado incompleto!\n");
        saida[0] = '\0';
        return;
    }

    // Processa pares de caracteres hexadecimais
    for (size_t i = 0; i < tam_cripto; i += 2)
    {
        unsigned char c1 = (unsigned char)criptografado[i];
        unsigned char c2 = (unsigned char)criptografado[i + 1];

        // Verifica se são dígitos hex válidos
        if (!isxdigit(c1) || !isxdigit(c2))
        {
            printf("Erro: Hex invalido!\n");
            saida[0] = '\0';
            return;
        }

        // Converte 2 caracteres hex para byte (usando strtoul que converte a string e unsigned long aceitando base hex)
        char byte_str[3] = {(char)c1, (char)c2, '\0'};
        unsigned long val = strtoul(byte_str, NULL, 16);
        unsigned char x = (unsigned char)val;

        // Verifica limite do buffer
        if (k + 1 >= MAX_1)
            break;

        // Aplica XOR reverso usando a mesma senha
        saida[k] = (char)(x ^ (unsigned char)senha[k % tam_psw]);
        k++;
    }

    // Termina string
    if (k < MAX_1)
        saida[k] = '\0';
    else
        saida[MAX_1 - 1] = '\0';
}

// ============================================================
// Gerador de senhas aleatórias
// ============================================================
void RandomPassword(char *senha)
{
    int tam_psw;

    // Solicita tamanho da senha com validação
    do
    {
        printf("Digite o tamanho da senha(1 - %d): ", MAX_PASSWORD - 1);

        if (scanf("%d", &tam_psw) != 1)
        {
            printf("Entrada invalida!\n");
            clear_stdin();
            continue;
        }

        if (tam_psw >= 1 && tam_psw <= (MAX_PASSWORD - 1))
            break;

        printf("Valor invalido!\n");

    } while (1);

    clear_stdin();

    // Cria array com caracteres ASCII imprimíveis (33 a 126)
    char ascii[94];
    for (int i = 0; i < 94; i++)
        ascii[i] = (char)(33 + i);

    // Embaralha o array usando Fisher-Yates(Todas as permutações tem mesma probabilidade)
    for (int i = 93; i > 0; i--)
    {
        int r = rand() % (i + 1);
        char t = ascii[i];
        ascii[i] = ascii[r];
        ascii[r] = t;
    }

    // Gera senha aleatória
    for (int i = 0; i < tam_psw; i++)
    {
        // Escolhe caractere aleatório do array embaralhado
        senha[i] = ascii[rand() % 94];
    }

    senha[tam_psw] = '\0'; // Termina string

    // Copia para clipboard e mostra na tela
    Copy_to_clipboard(senha);

    printf("Senha gerada e copiada para o Clipboard!\n");
    printf("Senha: %s\n", senha);
}

// ============================================================
// Salvar resultado em arquivo de texto
// ============================================================
void Salva_txt(const char *saida)
{
    char nome[256];
    // clear_stdin();  // já controlado pelo chamador

    printf("Nome do arquivo: ");

    if (!fgets(nome, sizeof(nome), stdin))
        strcpy(nome, "output"); // Nome padrão

    nome[strcspn(nome, "\n")] = '\0';
    trim_inplace(nome);

    if (strlen(nome) == 0)
        strcpy(nome, "output");

    // Adiciona extensão .txt
    char nome_f[300];
    snprintf(nome_f, sizeof(nome_f), "%s.txt", nome);

    // Abre arquivo para escrita
    FILE *arq = fopen(nome_f, "w");
    if (!arq)
    {
        printf("Erro ao criar arquivo\n");
        return;
    }

    // Escreve conteúdo
    fprintf(arq, "%s\n", saida);
    fclose(arq);

    // Obtém e mostra caminho completo do arquivo
    char caminho[500];
    GetFullPathNameA(nome_f, (DWORD)sizeof(caminho), caminho, NULL);

    printf("Arquivo salvo em:\n%s\n", caminho);
}

// ============================================================
// Copiar string para área de transferência (Clipboard)
// ============================================================
void Copy_to_clipboard(const char *str)
{
    if (!str)
        return;

    // Converte de UTF-8 para wide string (Unicode)
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (wlen == 0)
        return;

    // Aloca memória global (requisito do Windows Clipboard)
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wlen * sizeof(wchar_t));
    if (!hMem)
        return;

    wchar_t *pw = (wchar_t *)GlobalLock(hMem);
    if (!pw)
    {
        GlobalFree(hMem);
        return;
    }

    MultiByteToWideChar(CP_UTF8, 0, str, -1, pw, wlen);
    GlobalUnlock(hMem);

    // Abre e manipula clipboard
    if (!OpenClipboard(NULL))
    {
        // limpa memória alocada caso não consiga abrir o clipboard
        GlobalFree(hMem);
        return;
    }

    EmptyClipboard();
    // SetClipboardData toma posse do bloco de memória (não liberar depois)
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
}

// ============================================================
// Validação das entradas do usuário
// ============================================================
void valida_entrada(char *str, size_t max_len)
{
    while (1)
    {
        // Verifica se não está vazio e não excede limite (max_len inclui terminador)
        if (str && strlen(str) > 0 && strlen(str) < max_len)
            return;

        // Solicita nova entrada se inválida
        printf("Entrada invalida ou muito longa! Digite novamente (max %lu chars): ", (unsigned long)(max_len - 1));

        if (!fgets(str, (int)max_len, stdin))
        {
            str[0] = '\0';
            return;
        }

        str[strcspn(str, "\n")] = '\0';
        trim_inplace(str);
    }
}
