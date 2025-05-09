//INTEGRANTES
//Erivaldo José
//Guilherme Alessander
//João Victor 
//Leandro Marcio <--- Nome usado como base

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define MAXR 4 //n° max de restos
#define PRAZO_MAX 12 //o maximo de peridos para qualquer aluno é 12
#define MAX_DISC 3 //de acordo com o nome esse é o maximo de disciplina
#define MAX_REQUISITOS 10 //o máximo de requisitos por disciplina
//char* armazena um nome (uma string)
//char** armazena muitas strings (um array de string)

typedef struct {
    wchar_t* nome;
    int id;
    int carga;
    int peso;
    int periodo; //no caso da eletiva 0
    int tipo; //0 para obirgatoria, 1 para eletiva
    int lab; //caso precise de laboratoria
    wchar_t* horario;
    wchar_t** requisitos;
} Disciplina;

typedef struct {
    wchar_t* nome;
    wchar_t* formacao;
    int carga;
    int num_disciplinas; //qtd de disciplinas que esta
    wchar_t** especializacao;
    int disponibiidade[6][12]; //seis dias e 12 horarios de aula
} Professor;

typedef struct {
    wchar_t* codigo;
    int capacidade;
    int eh_lab; //0 se nao, 1 se sim
    int** disponibilidade;
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
    int qtd_prof;
    Sala** salas;
    Aluno** alunos;
    int qtd_alunos;
    Disciplina** disciplinas;
} Curso;

//parte para alocação

//parte para prioridades

//parte para estrate

//inicializar a disponibilidade da sala
int **inicializardisponibilidade() {
    int **disponibilidade = malloc(6 * sizeof(int*));
    if (!disponibilidade) return NULL;

    for (int i = 0; i < 6; i++) {
        disponibilidade[i] = calloc(12, sizeof(int));
        if (!disponibilidade[i]) {
            for (int j = 0; j < i; j++) free(disponibilidade[j]);
            free(disponibilidade);
            return NULL;
        }
    }
    return disponibilidade;
}

Sala** inicializar_salas_fixas() {
    Sala** salas = (Sala**)malloc(21 * sizeof(Sala*));
    if (!salas) {
        wprintf(L"Erro ao alocar memória para o array de salas.\n");
        return NULL;
    }

    const wchar_t* nomes_originais[21] = {
        L"Sala de Aula 01", L"Sala de Aula 02", L"Sala de Aula 03",
        L"Mini-sala 01", L"Mini-auditório", L"Sala de Reuniões",
        L"Laboratório de Robótica", L"Laboratório de Graduação 01", L"Laboratório de Graduação 02",
        L"Laboratório de Graduação 03", L"Lab. de Circ. Elétricos e Eletrônicos",
        L"Laboratório da Pós 01", L"Laboratório da Pós 02",
        L"Sala de Aula da Pós 01", L"Sala de Aula da Pós 02",
        L"Sala de Aula 207", L"Sala de Aula 206", L"Sala de Aula 205",
        L"Sala de Aula 204", L"Auditório", L"Sala de Reuniões"
    };

    int capacidades[21] = {
    40, 40, 40, 20, 60, 15, 25, 30, 30, 30, 20, 20, 20, 25, 25, 40, 40, 40, 40, 100, 15
};

int eh_lab[21] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
};

    for (int i = 0; i < 21; i++) {
        salas[i] = malloc(sizeof(Sala));
        if (!salas[i]) {
            liberar_salas(salas);
            return NULL;
        }
        salas[i]->codigo = wcsdup_safe(nomes_originais[i]);
        if (!salas[i]->codigo) {
            liberar_salas(salas);
            return NULL;
        }

        salas[i]->capacidade = capacidades[i];
        salas[i]->eh_lab = eh_lab[i];
        salas[i]->disponibilidade = inicializardisponibilidade();
        if (!salas[i]->disponibilidade) {
            liberar_salas(salas);
            return NULL;
        }
    }
    return salas;
}

//Liberar memoria das salas

void liberar_salas(Sala** salas) {
    if (!salas) return;
    for (int i = 0; i < 21; i++) {
        if (!salas[i]) continue;
        for (int j = 0; j < 6; j++) free(salas[i]->disponibilidade[j]);
        free(salas[i]->disponibilidade);
        free(salas[i]->codigo);
        free(salas[i]);
    }
    free(salas);
}


//FUNÇÃO PARA LISTAR HORARIOS OCUPADOS

void listar_horarios_ocupados(Sala *S) {
    if (!S) return;
    wprintf(L"\nHorários ocupados da sala %ls:\n", S->codigo);
    for (int dia = 0; dia < 6; dia++) {
        int ocupado = 0;
        for (int aula = 0; aula < 12; aula++) {
            if (S->disponibilidade[dia][aula] == 1) {
                if (!ocupado) {
                    wprintf(L" Dia %d: ", dia);
                    ocupado = 1;
                }
                wprintf(L"%d ", aula);
            }
        }
        if (ocupado) wprintf(L"\n");
    }
}
void listar_salas_disponiveis(Sala** salas, int dia, int aula) {
    if (!salas || dia < 0 || dia >= 6 || aula < 0 || aula >= 12) {
        wprintf(L"Parâmetros inválidos.\n");
        return;
    }

    wprintf(L"\nSalas disponíveis para o Dia %d, Aula %d:\n", dia, aula);
    int encontrou = 0;

    for (int i = 0; i < 21; i++) {
        Sala *s = salas[i];
        if (s->disponibilidade[dia][aula] == 0) {
            wprintf(L" - %ls (Capacidade: %d, Tipo: %ls)\n", 
                    s->codigo, s->capacidade, s->eh_lab ? L"Laboratório" : L"Comum");
            encontrou = 1;
        }
    }

    if (!encontrou)
        wprintf(L"Nenhuma sala disponível nesse horário.\n");
}

int marcarHorario(Sala* s, int dia, int aula) {
    if (!s || dia < 0 || dia >= 6 || aula < 0 || aula >= 12)
        return 0;
    if (s->disponibilidade[dia][aula]) return 0;
    s->disponibilidade[dia][aula] = 1;
    return 1;
}

//função para duplicar string, ajuda a não apontar p memória compartilhada
wchar_t* wcsdup_safe(const wchar_t* src) {
    if (!src) return NULL;
    wchar_t* copia = malloc((wcslen(src) + 1) * sizeof(wchar_t));
    if (copia) wcscpy(copia, src);
    return copia;
}

//função para identificar a maneira como os requistos são separados para poder incluir
wchar_t** dividir_requisitos(const wchar_t* linha) {
    wchar_t** lista = malloc(MAX_REQUISITOS * sizeof(wchar_t*));
    int count = 0;
    wchar_t* copia = wcsdup_safe(linha);
    wchar_t* token;
    wchar_t* context;

    token = wcstok(copia, L"_", &context);
    while (token && count < MAX_REQUISITOS) {
        lista[count++] = wcsdup_safe(token);
        token = wcstok(NULL, L"_", &context);
    }
    lista[count] = NULL;
    free(copia);
    return lista;
}

//Aqui ele verifica no arquivo de texto cada valor como periodo, CH e separa através das virgulas no arquivo
void extrair_valor(const wchar_t* linha, const wchar_t* chave, wchar_t* destino) {
    const wchar_t* inicio = wcsstr(linha, chave);
    if (!inicio) {
        destino[0] = L'\0';
        return;
    }
    inicio += wcslen(chave);
    while (*inicio == L' ') inicio++; // pula espaços

    const wchar_t* fim = wcschr(inicio, L',');
    if (!fim) fim = linha + wcslen(linha); // até o final

    wcsncpy(destino, inicio, fim - inicio);
    destino[fim - inicio] = L'\0';
}

// Leitura principal do arquivo de texto para poder organizar na struct
Disciplina** ler_disciplinas(const wchar_t* nome_arquivo, int* total) {
    setlocale(LC_ALL, "");
    FILE* arquivo = _wfopen(nome_arquivo, L"r, ccs=UTF-8");
    if (!arquivo) {
        wprintf(L"Erro ao abrir o arquivo.\n");
        return NULL;
    }

    wchar_t linha[512];
    Disciplina** lista = NULL;
    *total = 0;

    while (fgetws(linha, sizeof(linha) / sizeof(wchar_t), arquivo)) {
        linha[wcscspn(linha, L"\n")] = L'\0';

        wchar_t buffer[100];

        Disciplina* nova = malloc(sizeof(Disciplina));

        extrair_valor(linha, L"Nome:", buffer);
        nova->nome = wcsdup_safe(buffer);

        extrair_valor(linha, L"CH:", buffer);
        nova->carga = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Periodo:", buffer);
        nova->periodo = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Peso:", buffer);
        nova->tipo = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Lab:", buffer);
        nova->lab = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Horario:", buffer);
        nova->horario = wcsdup_safe(buffer);

        extrair_valor(linha, L"Requisito:", buffer);
        nova->requisitos = dividir_requisitos(buffer);

        lista = realloc(lista, (*total + 1) * sizeof(Disciplina*));
        lista[*total] = nova;
        (*total)++;
    }

    fclose(arquivo);
    return lista;
}

//Função p mostrar como ficou organizado as disciplinas, mas pode apagar se n for necessário para apresentação
void imprimir_disciplinas(Disciplina** disciplinas, int total) {
    for (int i = 0; i < total; i++) {
        Disciplina* d = disciplinas[i];
        wprintf(L"\nNome: %ls\n", d->nome);
        wprintf(L"Periodo: %d | Carga: %d | Peso: %d | Lab: %d\n", d->periodo, d->carga, d->tipo, d->lab);
        wprintf(L"Horário: %ls\n", d->horario);
        wprintf(L"Requisitos: ");
        if (d->requisitos) {
            for (int j = 0; d->requisitos[j]; j++) {
                wprintf(L"%ls ", d->requisitos[j]);
            }
        } else {
            wprintf(L"Nenhum");
        }
        wprintf(L"\n-----------------------------\n");
    }
}



//parte para alocação

//parte para prioridades
int decisaoOfertaDisc(Disciplina* disciplina, Aluno** alunos, int num_alunos, int periodoMax) {
    //contar quantos alunos podem cursar tal disci
    int interessados = 0;  //querem fazer a disc
    int no_prazo; //estao no prazo do curso

    for (int i = 0; i < num_alunos; i++) {
        int feita = 0; //se ja fex a materia
        for (int j = 0; alunos[i]->disciplinas_feitas[j] != NULL; j++) {
            if (wcscmp(alunos[i]->disciplinas_feitas[j], disciplina->id) == 0) {
                feita = 1;
                break;
            }
        }
        if (feita) continue;

        //prerequisito
        int tem_pre;
        for (int k = 0; disciplina->requisitos[k] != NULL; k++) {
            int legal = 0; //se tem pres
            for (int l = 0; alunos[i]->disciplinas_feitas[l] != NULL; l++) {
                if (wcscmp(alunos[i]->disciplinas_feitas[l], disciplina->requisitos[k]) == 0) {
                    legal = 1;
                    break;
                }
            }

            if (!legal) {
                tem_pre = 0;
                break;
            }
        }

        if (tem_pre) {
            interessados++;
            if (alunos[i]->periodo <= periodoMax) { //verifica perido
                no_prazo++;
            }
        }
    }

    //nnoa ofertamento
    if (interessados == 0) return 0;
    if(interessados < 10 && no_prazo == 0) return printf("Sem alunos minimos ou aluno no prazo do curso para ofertar\n");

    return 1; // verdadeiro
  
}

//achar prof qualificado
Professor** buscarProfQualif(Professor** professores, int num_prof, Disciplina* disciplina, int* prof_achados) {
    Professor** qualificados = malloc(num_prof * sizeof(Professor*));
    *prof_achados = 0; //contador para professor aptos

    for (int i = 0; i < num_prof; i++) {
        if (disciplina->periodo <= 4) {
            qualificados[(*prof_achados)++] = professores[i];
            continue;
        }
        int especializado = 0; //sobre cada especialização
        for (int j = 0;professores[i]->especializacao[j] != NULL; j++) {
            //verfiica a especializaçao do professor, ainda esta simples
            if (wcsstr(professores[i]->especializacao[j], L"Computação") != NULL ||
                wcsstr(professores[i]->especializacao[j], L"Enegnharia") != NULL) {
                    especializado = 1;
                    break;
            }
        } 

        if (especializado) {
            qualificados[(*prof_achados)++] = professores[i];
        }
    }

    return qualificados;
}

//funcao para ofertar as disciplinas 
void ofertarDisc(Curso* curso) {
    //processar obrigatoriad
    for (int i = 0; curso->disciplinas[i] != NULL; i++) {
        Disciplina* disc = curso->disciplinas[i];

        //obg ou enfase
        if (disc->tipo == 0 || disc->peso > 0) {
            //ofertar ou nnao
            if (decisaoOfertaDisc(disc, curso->alunos, curso->qtd_alunos, PRAZO_MAX)) {
                int n_prof; // achar prof qualifcd
                Professor** qualificados = buscarProfQualif(curso->professores, curso->qtd_prof, disc, &n_prof);

                if (n_prof > 0) {
                    //alocacao
                    wprintf(L"Disciplina %ls será ofertada\n", disc->nome);
                } else {
                    printf("De acordo com os criterios: Considere a possibilidade de solicitar um professor de outro instituto para lecionar a disciplina\n");
                }

                free(qualificados);
            }
        } else wprintf(L"Disciplina %ls noa pode ser ofertada, pois nao tem os criterios necessarios\n"); 
    }

    //FALTA ELETIVAS
}


//parte para estrategias de ofertas e etc

void Situacao (int resto[], Aluno* aluno) {//essa função descreve os critérios estabelecidos pela professora{
    wprintf(L"=============================CRITÉRIOS=============================\n");
    
    wprintf(L"-> Máximo de disciplinas por semestre será de ");
    
    switch (resto[0]){
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

    switch (resto[1]) {
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

    wprintf(L"-> O critério de alocação de professoress será: ");

    switch (resto[2]) {
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

    wprintf(L"-> A ferta das disciplinas se dará: ");

    switch (resto[3]) { 
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


//parte para auxiliares e carregamento
//carregar dados do curos nos arquivos de textos
/*Curso* carregarCurso(const char* arq_disc, const char* arq_prof, const char* arq_aluno) {
    Curso* curso = malloc(sizeof(Curso));


} */

//parte para validar
int value_string(wchar_t letra) { //retorna o valor de cada letra do nome 
   switch (letra) {
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
int name_sum(wchar_t *nome) { //soma os valores das letras
   int soma = 0;

   for (int i = 0; nome[i] != L'\0'; ++i)
   {
        soma += value_string(towlower(nome[i]));
   }
   
   return soma;
}

//função para separação do nome em partes para fazer a divisão
void name_process(Aluno aluno, int resto[]) {
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
        wchar_t * token = wcstok(copiaNome, delimitadores, &ultimaParada); 

        while (token != NULL) { //vai separar e ler cada partição, ou palavra, do nome
            if (j > 3) break;//para caso o nome da pessoa seja muito extenso
     
            if (wcslen(token) > 3) {//caso o tamanho da palavra for <= 3 a condição irá ignorar essa palavra e vai pular para a próxima
                soma = name_sum(token);
                resto[j] = (soma % 3);
                //wprintf(L"%d° palavra do nome: %ls, tem %ld letras e a soma das suas letras eh: %d\n", j + 1, token, wcslen(token), soma);
                j++;
            }
    
            token = wcstok(NULL, delimitadores, &ultimaParada);  //esse NULL é para dizer para ela continuar o processo
        }        
    }
    
    return;
}

//main
int main() {
    setlocale(LC_ALL, "");
    fwide(stdout, 1);
    
    Aluno aluno = {.nome = L"Leandro Marcio Elias da Silva"};

    int resto[MAXR]; 

    name_process(aluno, resto);
    Situacao(resto, &aluno);

    Curso* curso = carregarCurso("disciplinas.txt", "professores.txt", "alunos.txt");
    ofertarDisc(curso);
    free(curso);

    Sala** salas = inicializar_salas_fixas();

    if(!salas){
        wprintf(L"Erro ao inicializar salas fixas.\n");
        return 1;

    }


    int opcao;
    do {
        wprintf(L"\n=== MENU SALAS ===\n");
        wprintf(L"1. Listar horários ocupados de uma sala\n");
        wprintf(L"2. Listar salas disponíveis em um horário\n");
        wprintf(L"3. Marcar um horário como ocupado (reserva de sala)\n");
        wprintf(L"4. Alocar disciplina automaticamente\n");
        wprintf(L"0. Sair\n");
        wprintf(L"Escolha uma opção: ");
        wscanf(L"%d", &opcao);
        getchar();

        switch (opcao) {
            case 1: {
                int num;
                wprintf(L"Digite o número da sala (0 a 20): ");
                wscanf(L"%d", &num);
                getchar();
                if (num < 0 || num >= 21) {
                    wprintf(L"Número de sala inválido.\n");
                    break;
                }
                listar_horarios_ocupados(salas[num]);
                break;
            }
            case 2: {
                int dia, aula;
                wprintf(L"Digite o dia da semana (0=Seg, ..., 5=Sáb): ");
                wscanf(L"%d", &dia);
                wprintf(L"Digite o número da aula (0 a 11): ");
                wscanf(L"%d", &aula);
                getchar();
                listar_salas_disponiveis(salas, dia, aula);
                break;
            }
            case 3: {
                int num, dia, aula;
                wprintf(L"Digite o numero da sala (0 a 20): ");
                wscanf(L"%d", &num);
                wprintf(L"Digite o dia da semana (0=Seg, ..., 5=Sáb): ");
                wscanf(L"%d", &dia);
                wprintf(L"Digite o número da aula (0 a 11): ");
                wscanf(L"%d", &aula);
                getchar();
                if(marcarHorario(salas[num], dia, aula))
                {
                    wprintf(L"Horario marcado com sucesso\n");
                }else
                {
                    wprintf(L"Horario ja ocupado ou invalido\n");

                }
                break;
            }
            case 4: {
              wchar_t nome_disciplina[100];
    int capacidade_minima, lab, dia, aula;
    wprintf(L"Nome da disciplina: ");
    fgetws(nome_disciplina, sizeof(nome_disciplina) / sizeof(wchar_t), stdin);
    nome_disciplina[wcscspn(nome_disciplina, L"\n")] = L'\0';

    wprintf(L"Capacidade mínima: ");
    wscanf(L"%d", &capacidade_minima);
    wprintf(L"Necessita laboratório? (1 = Sim, 0 = Não): ");
    wscanf(L"%d", &lab);
    wprintf(L"Dia da semana (0=Seg, ..., 5=Sáb): ");
    wscanf(L"%d", &dia);
    wprintf(L"Número da aula (0 a 11): ");
    wscanf(L"%d", &aula);
    getchar();

    int sucesso = 0;
    for (int i = 0; i < 21; i++) {
        Sala *s = salas[i];
        if (s->capacidade >= capacidade_minima && s->eh_lab == lab && s->disponibilidade[dia][aula] == 0) {
            s->disponibilidade[dia][aula] = 1;
            wprintf(L"[OK] Disciplina \"%ls\" alocada na sala %ls.\n", nome_disciplina, s->codigo);
            sucesso = 1;
            break;
        }
    }

    if (!sucesso){
        wprintf(L"[Aviso] Nenhuma sala atende aos critérios.\n");
    break;
                }
            }
            case 0:
                wprintf(L"Encerrando o programa...\n");
                break;
            default:
                wprintf(L"Opção inválida.\n");
        }

    } while (opcao != 0);

  liberar_salas(salas);


   
    
    return 0;
}
