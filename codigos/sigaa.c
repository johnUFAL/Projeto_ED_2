//INTEGRANTES
//Erivaldo José
//Leandro Marcio
//Guilherme Alessander
//João Victor <--- Nome usado como base

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

//char* armazena um nome (uma string)
//char** armazena muitas strings (um array de string)

typedef struct {
    wchar_t* nome;
    int carga;
    int perido; //no caso da eletiva 0
    int tipo; //0 para obirgatoria, 1 para eletiva
    int lab; //caso precise de laboratoria
    wchar_t** requisitos;
} Disciplina;

typedef struct {
    wchar_t* nome;
    wchar_t* formacao;
    int carga;
    int num_disciplinas; //qtd de disciplinas que esta
    wchar_t** especialização;
    int disponibiidade[6][12]; //seis dias e 12 horarios de aula
} Professor;

typedef struct {
    wchar_t* codigo;
    int capacidade;
    int eh_lab; //0 se nao, 1 se sim
    int disponibiidade[6][12];
} Sala;

typedef struct {
    wchar_t* nome;
    int matricula;
    int periodo;
    wchar_t** disciplinas_feitas;
    wchar_t** disciplinas_falta;
} Aluno;

typedef struct {
    Disciplina* disciplina;
    Professor* professor;
    Sala* sala;
    wchar_t* horario;
    wchar_t* semestre;
    Aluno** matriculados;
    int qtd;
} Oferta;

typedef struct {
    wchar_t* nome;
    Oferta** ofertas;
    Professor** professores;
    Sala** salas;
    Aluno** alunos;
    Disciplina** disciplinas;
} Curso;

//parte para alocação

//parte para prioridades

//parte para estrategias de ofertas e etc

//parte para auxiliares e carregamento

//parte para validar

void Situacao (int resto[]) //essa função descreve os critérios estabelecidos pela professora
{
    wprintf(L"=============================CRITÉRIOS=============================\n");
    
    wprintf(L"-> Máximo de disciplinas por semestre será de ");
    
    switch (resto[0])
    {
        case 0:
            wprintf(L"3 disciplinas\n");
            break;

        case 1:
            wprintf(L"2 disciplinas\n");    
            break;

        case 2:
            wprintf(L"1 disciplinas\n");
            break;

        default:
            wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> A solicitação de professores será: ");

    switch (resto[1])
    {
        case 0:
            wprintf(L"Considere a possibilidade de solicitar um professor de outro instituto para lecionar a disciplina\n");
            break;

        case 1:
            wprintf(L"Considere a possibilidade de solicitar um professor substituto para lecionar a disciplina\n");  
            break;

        case 2:
            wprintf(L"Considere a possibilidade de dividir a disciplina entre mais de um professor do IC para lecionar a disciplina\n");
            break;

        default:
            wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> O critério de alocação de professoress será ");

    switch (resto[2])
    {
        case 0:
            wprintf(L"Os professores deve ser alocados no menor números de dias possíveis\n");
            break;

        case 1:
            wprintf(L"Os professores deve ser alocados no menor números de dias possíveis\n");    
            break;

        case 2:
            wprintf(L"Os professores deve ser alocados de modo a ir ao IC todos os dias\n");
            break;

        default:
            wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> A ferta das disciplinas se dará ");

    switch (resto[3])
    { 
        case 0:
            wprintf(L"As disciplinas com pré requisitos tem maior prioridade\n");
            break;

        case 1:
            wprintf(L"As disciplinas obrigatórias devem ter maior prioridade\n");    
            break;

        case 2:
            wprintf(L"As disciplinas de ênfase devem ter maior prioridade\n");
            break;

        default:
            wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }
    return;
}

int value_string(wchar_t letra) //retorna o valor de cada letra do nome
{ 
   switch (letra) 
   {
       case L'q': return 1; case L'w': return 6; case L'e': return 7;
       case L'r': return 6; case L't': return 5; case L'y': return 2;
       case L'u': return 3; case L'i': return 8; case L'o': return 9;
       case L'p': return 4; case L'á': return 3; case L'ã': return 4;
       case L'a': return 2; case L's': return 5; case L'd': return 8;
       case L'f': return 7; case L'g': return 4; case L'h': return 1;
       case L'j': return 4; case L'k': return 7; case L'l': return 8;
       case L'ç': return 5; case L'é': return 2; case L'í': return 3;
       case L'z': return 3; case L'x': return 4; case L'c': return 9;
       case L'v': return 8; case L'b': return 3; case L'n': return 2;
       case L'm': return 5; case L'ó': return 6; case L'õ': return 7;
       case L'ô': return 6; case L'â': return 1; case L'ê': return 2;
       default: return 0;
   }
}

//parte para processar nome
int name_sum(wchar_t *nome) //soma os valores das letras
{
   int soma = 0;

   for (int i = 0; nome[i] != L'\0'; ++i)
   {
        soma += value_string(towlower(nome[i]));
   }
   
   return soma;
}

//função para separação do nome em partes para fazer a divisão
void name_process(Aluno aluno, int resto[]) 
{
    wchar_t copiaNome[60];
    wchar_t * ultimaParada; //ponteiro que guarda a posição de onde a função wcstok parou
    wchar_t * delimitadores = L" "; //ponteiro que armazena os delimitadores da função wcstok que nesse caso é somente o espaço

    int j = 0; //indice para o array de inteiros e limitador de palavras
    int soma = 0; //guardará a soma das letras 
    
    while (j < 4) //loop para caso o nome seja pequeno e não consiga suprir os requisitos
    {
        wcscpy(copiaNome, aluno.nome); //apesar de aluno.nome ser uma cópia iremos criar mais uma cópia por prevenção
        
        //retorna uma substring da string nome
        //recebe uma string, seus delimitadores e a última posição do ponteiro que é inicialmente NULL
        wchar_t *token = wcstok(copiaNome, delimitadores);

        while (token != NULL) //vai separar e ler cada partição, ou palavra, do nome
        {
            if (j > 3) //para caso o nome da pessoa seja muito extenso
            {
                break;
            }
    
            if (wcslen(token) > 3) //caso o tamanho da palavra for <= 3 a condição irá ignorar essa palavra e vai pular para a próxima
            {
                soma = name_sum(token);
                resto[j] = (soma % 3);
                //wprintf(L"%d° palavra do nome: %ls, tem %ld letras e a soma das suas letras eh: %d\n", j + 1, token, wcslen(token), soma);
                j++;
            }
    
            token = wcstok(NULL, delimitadores); //esse NULL é para dizer para ela continuar o processo
        }        
    }
    
    return;
}

//main
#define MAXR 4 //n° max de restos

int main() {
    setlocale(LC_ALL, "");
    fwide(stdout, 1);
    
    Aluno aluno = {.nome = L"João Victor Duarte do Nascimento"};

    int resto[MAXR]; 

    name_process(aluno, resto);
    Situacao(resto);
    
    return 0;
}