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
#include <wchar.h>
#include <wctype.h>

//coisa do LINUX, aparentemente precisa disso
#ifndef _GNU_SOURCE
#define _GNU_SOURCE  // Para wcsdup no Linux
#endif

//compatilibilade com win
#ifdef _WIN32
#define wcsdup _wcsdup
#define wcstol _wtol
#endif

#define MAXR 4 //n° max de restos
#define PRAZO_MAX 12 //o maximo de peridos para qualquer aluno é 12
#define MAX_DISC 3 //de acordo com o nome esse é o maximo de disciplina
#define MAX_REQUISITOS 10 //o máximo de requisitos por disciplin

//char* armazena um nome (uma string)
//char** armazena muitas strings (um array de string)

typedef struct {
    wchar_t* nome;
    wchar_t* id;
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
    int disponibilidade[6][12]; //seis dias e 12 horarios de aula
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

//o que tiver qtd provavelmente eh um contador

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
    int qtd_salas;
    Aluno** alunos;
    int qtd_alunos;
    Disciplina** disciplinas;
    int qtd_disciplinas;  
    int qtd_ofertas;
} Curso;

//criacao de salas
Sala* criarSala(const wchar_t* codigo, int capacidade, int eh_lab){
//o codigo é constante porque codigo de disciplina não se altera aqui
    
    Sala* S = (Sala*)malloc(sizeof(Sala));

    if(!S){
        wprintf(L"Erro ao alocar sala dinamicamente");
        return NULL;
    }

     S->codigo = (wchar_t*)malloc((wcslen(codigo) + 1) * sizeof(wchar_t));
    if(!S->codigo){

        wprintf(L"Erro ao alocar codigo dinamicamente");
        free(S);
        return NULL;
    }

    wcscpy(S->codigo, codigo);

    S->capacidade = capacidade;
    S->eh_lab = eh_lab;

    //vamos alocar matriz de disponibilidade agora
    S->disponibilidade = (int**)malloc(6*sizeof(int*));

    if(!S->disponibilidade){
        wprintf(L"Erro ao alocar disponibilidade dinamicamente");
        free(S->codigo);
        free(S);
        return NULL;
    }

    for(int i = 0; i < 6; i++){
        S->disponibilidade[i] = (int*)calloc(12, sizeof(int)); //inicializa 0, ou seja livre

          if(!S->disponibilidade[i]){
            wprintf(L"Erro ao alocar memoria");

            //libera a memoria ja alocada antes de retornar

            for(int j = 0; j < i; j++){
                free(S->disponibilidade[j]);
            }
              free(S->disponibilidade);
              free(S->codigo);
              free(S);
              return NULL;
          }
    }
    return S;
}

int marcarHorario(Sala* S, int dia, int aula) {
    if (!S || dia < 0 || dia >= 6 || aula < 0 || aula >= 12) return 0;
    if (S->disponibilidade[dia][aula] == 1) return 0; // já ocupado

    S->disponibilidade[dia][aula] = 1; // marcar como ocupado
    return 1;
}

//desicao das ofertas de disciplinas
int decisaoOfertaDisc(Disciplina* disciplina, Aluno** alunos, int num_alunos, int periodoMax) {
    //contar quantos alunos podem cursar tal disci
    int interessados = 0;  //querem fazer a disc
    int no_prazo = 0; //estao no prazo do curso

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
        int tem_pre = 1;
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
    if(interessados < 10 && no_prazo == 0) return wprintf(L"Sem alunos minimos ou aluno no prazo do curso para ofertar\n");

    return 1; // verdadeiro
  
}

//achar prof qualificado
Professor** buscarProfQualif(Professor** professores, int num_prof, Disciplina* disciplina, int* prof_achados) {
    Professor** qualificados = malloc(num_prof * sizeof(Professor*));
    *prof_achados = 0; //contador para professor aptos

    for (int i = 0; i < num_prof; i++) {
        //as basicas, perido 4 pra baixo (ED ne basico nao oxi)
        if (disciplina->periodo <= 4) {
            qualificados[(*prof_achados)++] = professores[i];
            continue;
        }
        for (int j = 0; professores[i]->especializacao[j] != NULL; j++) {
            //verfiica a especializaçao do professor, ainda esta simples
            if (wcsstr(professores[i]->especializacao[j], L"Computação") != NULL ||
                wcsstr(professores[i]->especializacao[j], L"Engenharia") != NULL) {
                qualificados[(*prof_achados)++] = professores[i];
                break;
                }
            }
        } 
    return qualificados;
}

//funcao para ofertar as disciplinas com professor (2 funcao principal)
void ofertarDisc(Curso* curso) {
    //processar obrigatoriad
    for (int i = 0; curso->disciplinas[i] != NULL; i++) {
        
        wprintf(L"[DEBUG] Verificando disciplina %d\n", i);
        Disciplina* disc = curso->disciplinas[i];

        //obg ou enfase
        if (disc->tipo == 0 || disc->peso > 0) {
            wprintf(L"[DEBUG] Chamando decisaoOfertaDisc()\n");

            int pode_ofertar = decisaoOfertaDisc(disc, curso->alunos, curso->qtd_alunos, PRAZO_MAX);
            int prof_encontrado = 0;
            
            wprintf(L"[DEBUG] decisaoOfertaDisc retornou: %d\n", pode_ofertar);

            
            if (pode_ofertar) {
                wprintf(L"[DEBUG] Chamando buscarProfQualif()\n");

                Professor** quali = buscarProfQualif(curso->professores, curso->qtd_prof, disc, &prof_encontrado);
                
                wprintf(L"[DEBUG] buscarProfQualif retornou %d professor(es)\n", prof_encontrado);

                if (prof_encontrado > 0) {
                    //o primeiro prof qualificadfo eh o que eh
                    wprintf(L"\n----- Professores e sua(s) Disciplina(s) -----\n");
                    wprintf(L"%ls: %ls\n", quali[0]->nome, disc->nome);
                    wprintf(L"------------------------------------------------\n");
                } else {
                    wprintf(L"----- Professor Externo Necessario -----\n");
                    wprintf(L"Considere solicitar professor de outro instituto para: %ls\n", disc->nome);
                    wprintf(L"------------------------------------------------\n");
                }
                free(quali);
            } else {
                wprintf(L"[DEBUG] Disciplina não será ofertada\n");
                wprintf(L"\n----- Disciplina Nao Ofertada -----\n");
                wprintf(L"%ls nao atendeu as solicitacoes minimas\n", disc->nome);
                wprintf(L"------------------------------------------------\n");
            }
        }
    }    
}

//parte para estrategias de ofertas e etc
void Situacao (int resto[], Aluno* aluno) {//essa função descreve os critérios estabelecidos pela professora{
    wprintf(L"=============================CRITÉRIOS=============================\n");
    
    wprintf(L"-> Máximo de disciplinas por semestre será de ");
    
    switch (resto[0]){
        case 0: wprintf(L"3 disciplinas\n"); break;
        case 1: wprintf(L"2 disciplinas\n"); break;
        case 2: wprintf(L"1 disciplinas\n"); break;
        default: wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> A solicitação de professores será: ");

    switch (resto[1]) {
        case 0: wprintf(L"Considere a possibilidade de solicitar um professor de outro instituto para lecionar a disciplina\n");
            break;
        case 1: wprintf(L"Considere a possibilidade de solicitar um professor substituto para lecionar a disciplina\n");  
            break;
        case 2: wprintf(L"Considere a possibilidade de dividir a disciplina entre mais de um professor do IC para lecionar a disciplina\n");
            break;
        default: wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> O critério de alocação de professoress será: ");

    switch (resto[2]) {
        case 0: wprintf(L"Os professores deve ser alocados no menor números de dias possíveis\n");
            break;
        case 1: wprintf(L"Os professores deve ser alocados no menor números de dias possíveis\n");    
            break;
        case 2: wprintf(L"Os professores deve ser alocados de modo a ir ao IC todos os dias\n");
            break;
        default: wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> A oferta das disciplinas se dará: ");

    switch (resto[3]) { 
        case 0: wprintf(L"As disciplinas com pré requisitos tem maior prioridade\n");
            break;
        case 1: wprintf(L"As disciplinas obrigatórias devem ter maior prioridade\n");    
            break;
        case 2: wprintf(L"As disciplinas de ênfase devem ter maior prioridade\n");
            break;
        default: wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }
    return;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   

//parte para auxiliares e carregamento
void carregarDisc(const char* nome_arq, Disciplina*** disciplinas, int* cont) {
    FILE* file = fopen(nome_arq, "r, ccs=UTF-8");
    if (!file) {
        perror("Erro ao abrir arquivo de disciplinas");
        exit(1);
    }

    wchar_t linha[1000]; //SE TIVER MAIS MUDA AQUI!!!!!!!
    *cont = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Periodo:")) (*cont)++;
    }
    rewind(file);

    *disciplinas = (Disciplina**)malloc(*cont * sizeof(Disciplina*));
    if (!*disciplinas) {
        perror("Erro ao alocar disciplinas");
        fclose(file);
        exit(1);
    }

    int i = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Periodo:")) {
            Disciplina* d = (Disciplina*)malloc(sizeof(Disciplina));
            
            d->nome = (wchar_t*)malloc(100 * sizeof(wchar_t));
            d->id = (wchar_t*)malloc(10 * sizeof(wchar_t));
            d->horario = (wchar_t*)malloc(20 * sizeof(wchar_t));
            d->requisitos = (wchar_t**)malloc(5 * sizeof(wchar_t*));
            
            wchar_t* svptr;
            wchar_t* tk = wcstok(linha, L",", &svptr);
            while (tk != NULL) {
                if (wcsstr(tk, L"Periodo:")) {
                    swscanf(tk, L"Periodo: %d", &d->periodo);
                } else if (wcsstr(tk, L" Nome:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    wcscpy(d->nome, play);
                } else if (wcsstr(tk, L" Id:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    wcscpy(d->id, play);
                } else if (wcsstr(tk, L" Peso:")) {
                    swscanf(tk, L" Peso: %d", &d->peso);
                } else if (wcsstr(tk, L" CH:")) {
                    swscanf(tk, L" CH: %d", &d->carga);
                } else if (wcsstr(tk, L" Requisito:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    // Processar requisitos
                } else if (wcsstr(tk, L" Horario:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    wcscpy(d->horario, play);
                }
                tk = wcstok(NULL, L",", &svptr);
            }
            
            d->tipo = (d->periodo == 0) ? 1 : 0;
            d->lab = (wcsstr(d->nome, L"Programação") != NULL) ? 1 : 0;
            
            (*disciplinas)[i++] = d;
        }
    }
    fclose(file);
}

void carregarProf(const char* nome_arq, Professor*** professores, int* cont) {
    FILE* file = fopen(nome_arq, "r, ccs=UTF-8");
    if (!file) {
        perror("Erro ao abrir arquivo de professores");
        exit(1);
    }

    wchar_t linha[500];
    *cont = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) (*cont)++;
    }
    rewind(file);

    *professores = (Professor**)malloc(*cont * sizeof(Professor*));
    if (!*professores) {
        perror("Erro ao alocar professores");
        fclose(file);
        exit(1);
    }

    int i = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) {
            Professor* p = (Professor*)malloc(sizeof(Professor));
            
            p->nome = (wchar_t*)malloc(100 * sizeof(wchar_t));
            p->formacao = (wchar_t*)malloc(500 * sizeof(wchar_t));
            p->especializacao = (wchar_t**)malloc(10 * sizeof(wchar_t*));
            
            for (int d = 0; d < 6; d++) {
                for (int h = 0; h < 12; h++) {
                    p->disponibilidade[d][h] = 1;
                }
            }
            
            wchar_t* svptr;
            wchar_t* tk = wcstok(linha, L",", &svptr);
            while (tk != NULL) {
                if (wcsstr(tk, L"Nome:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1; 
                    while (*play == L' ') play++;
                    wcscpy(p->nome, play);
                } else if (wcsstr(tk, L" Formação:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(p->formacao, play);
                    
                    wchar_t* esp_saveptr;
                    wchar_t* esp_token = wcstok(p->formacao, L",", &esp_saveptr);
                    int esp_count = 0;
                    while (esp_token != NULL && esp_count < 10) {
                        p->especializacao[esp_count] = (wchar_t*)malloc(100 * sizeof(wchar_t));
                        wcscpy(p->especializacao[esp_count], esp_token);
                        esp_count++;
                        esp_token = wcstok(NULL, L",", &esp_saveptr);
                    }
                    p->especializacao[esp_count] = NULL;
                }
                tk = wcstok(NULL, L",", &svptr);
            }

            p->carga = 0;
            p->num_disciplinas = 0;
            (*professores)[i++] = p;
        }
    }
    fclose(file);
}

void carregarAluno(const char* nome_arq, Aluno*** alunos, int* cont) {
    FILE* file = fopen(nome_arq, "r, ccs=UTF-8");
    if (!file) {
        perror("Erro ao abrir arquivo de alunos");
        exit(1);
    }

    wchar_t linha[1000];
    *cont = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) (*cont)++;
    }
    rewind(file);

    *alunos = (Aluno**)malloc(*cont * sizeof(Aluno*));
    if (!*alunos) {
        perror("Erro ao alocar alunos");
        fclose(file);
        exit(1);
    }

    int i = 0;
    Aluno* atual_al = NULL;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) {
            if (atual_al != NULL) {
                (*alunos)[i++] = atual_al;
            }
            
            atual_al = (Aluno*)malloc(sizeof(Aluno));
            atual_al->nome = (wchar_t*)malloc(100 * sizeof(wchar_t));
            atual_al->disciplinas_feitas = (wchar_t**)malloc(50 * sizeof(wchar_t*));
            atual_al->disciplinas_falta = (wchar_t**)malloc(50 * sizeof(wchar_t*));
            
            wchar_t* svptr;
            wchar_t* tk = wcstok(linha, L",", &svptr);
            while (tk != NULL) {
                if (wcsstr(tk, L"Nome:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(atual_al->nome, play);
                } else if (wcsstr(tk, L" Periodo:")) {
                    swscanf(tk, L" Periodo: %d", &atual_al->periodo);
                }
                tk = wcstok(NULL, L",", &svptr);
            }
            
            atual_al->disciplinas_feitas[0] = NULL;
            atual_al->disciplinas_falta[0] = NULL;
        } 
        else if (wcsstr(linha, L"Id:")) {
            wchar_t id[20], nome[100];
            float nota;
            
            swscanf(linha, L"Id: %19[^,], Nome: %99[^,], Nota: %f", id, nome, &nota);
            
            if (nota >= 5.0) {
                int j = 0;
                while (atual_al->disciplinas_feitas[j] != NULL) j++;
                atual_al->disciplinas_feitas[j] = (wchar_t*)malloc(20 * sizeof(wchar_t));
                wcscpy(atual_al->disciplinas_feitas[j], id);
                atual_al->disciplinas_feitas[j+1] = NULL;
            } else {
                int j = 0;
                while (atual_al->disciplinas_falta[j] != NULL) j++;
                atual_al->disciplinas_falta[j] = (wchar_t*)malloc(20 * sizeof(wchar_t));
                wcscpy(atual_al->disciplinas_falta[j], id);
                atual_al->disciplinas_falta[j+1] = NULL;
            }
        }
    }
    
    if (atual_al != NULL) {
        (*alunos)[i++] = atual_al;
    }
    
    fclose(file);
}
void freeDisciplinas(Disciplina* d) {
    if (!d) return;
    if (d->nome) free(d->nome);
    if (d->id) free(d->id);
    if (d->horario) free(d->horario);
    if (d->requisitos) {
        for (int i = 0; d->requisitos[i]; i++) free(d->requisitos[i]);
        free(d->requisitos);
    }
    free(d);
}

void freeProf(Professor* p) {
    if (!p) return;
    if (p->nome) free(p->nome);
    if (p->formacao) free(p->formacao);
    if (p->especializacao) {
        for (int i = 0; p->especializacao[i]; i++) free(p->especializacao[i]);
        free(p->especializacao);
    }
    free(p);
}

void freeAluno(Aluno* a) {
    if (!a) return;
    if (a->nome) free(a->nome);
    if (a->disciplinas_feitas) {
        for (int i = 0; a->disciplinas_feitas[i]; i++) free(a->disciplinas_feitas[i]);
        free(a->disciplinas_feitas);
    }
    if (a->disciplinas_falta) {
        for (int i = 0; a->disciplinas_falta[i]; i++) free(a->disciplinas_falta[i]);
        free(a->disciplinas_falta);
    }
    free(a);
}

void freeSala(Sala* s) {
    if (!s) return;
    if (s->codigo) free(s->codigo);
    if (s->disponibilidade) {
        for (int i = 0; i < 6; i++) {
            free(s->disponibilidade[i]);
        }
        free(s->disponibilidade);
    }
    free(s);
}

void freeOferta(Oferta* o) {
    if (!o) return;
    if (o->horario) free(o->horario);
    if (o->semestre) free(o->semestre);
    if (o->matriculados) free(o->matriculados);
    free(o);
}

void freeCurso(Curso* curso) {
    if (curso == NULL) return;
    if (curso->nome != NULL) free(curso->nome);
    if (curso->disciplinas != NULL) {
        for (int i = 0; i < curso->qtd_disciplinas; i++) {
            freeDisciplinas(curso->disciplinas[i]);
        }
        free(curso->disciplinas);
    }
    
    if (curso->professores != NULL) {
        for (int i = 0; i < curso->qtd_prof; i++) {
            freeProf(curso->professores[i]);
        }
        free(curso->professores);
    }
    
    if (curso->alunos != NULL) {
        for (int i = 0; i < curso->qtd_alunos; i++) {
            freeAluno(curso->alunos[i]);
        }
        free(curso->alunos);
    }
    
    if (curso->salas != NULL) {
        for (int i = 0; i < curso->qtd_salas; i++) {
            freeSala(curso->salas[i]);
        }
        free(curso->salas);
    }
    
    if (curso->ofertas != NULL) {
        for (int i = 0; i < curso->qtd_ofertas; i++) {
            freeOferta(curso->ofertas[i]);
        }
        free(curso->ofertas);
    }
}

//função para identificar a maneira como os requistos são separados para poder incluir
wchar_t** dividir_requisitos(const wchar_t* linha) {
    wchar_t** lista = malloc(MAX_REQUISITOS * sizeof(wchar_t*));
    int count = 0;
    wchar_t* copia = wcsdup(linha);
    wchar_t* token;
    wchar_t* context;

    token = wcstok(copia, L"_", &context);
    while (token && count < MAX_REQUISITOS) {
        lista[count++] = wcsdup(token);
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
        nova->nome = wcsdup(buffer);

        extrair_valor(linha, L"CH:", buffer);
        nova->carga = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Periodo:", buffer);
        nova->periodo = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Peso:", buffer);
        nova->tipo = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Lab:", buffer);
        nova->lab = wcstol(buffer, NULL, 10);

        extrair_valor(linha, L"Horario:", buffer);
        nova->horario = wcsdup(buffer);

        extrair_valor(linha, L"Requisito:", buffer);
        nova->requisitos = dividir_requisitos(buffer);

        lista = realloc(lista, (*total + 1) * sizeof(Disciplina*));
        lista[*total] = nova;
        (*total)++;
    }

    fclose(arquivo);
    return lista;
}

int comparar_periodo_e_prereq(const void* a, const void* b) {
    Disciplina* d1 = *(Disciplina**)a;
    Disciplina* d2 = *(Disciplina**)b;

    // Período 0 é sempre o último
    if (d1->periodo == 0 && d2->periodo != 0) return 1;
    if (d1->periodo != 0 && d2->periodo == 0) return -1;

    // Ordena por período crescente
    if (d1->periodo != d2->periodo)
        return d1->periodo - d2->periodo;

    // Prioriza as que têm pré-requisitos
    int d1_tem_req = (d1->requisitos && d1->requisitos[0] && wcscmp(d1->requisitos[0], L"Nenhum") != 0);
    int d2_tem_req = (d2->requisitos && d2->requisitos[0] && wcscmp(d2->requisitos[0], L"Nenhum") != 0);

    return d2_tem_req - d1_tem_req;
}

int comparar_periodo_e_obrigatoriedade(const void* a, const void* b) {
    Disciplina* d1 = *(Disciplina**)a;
    Disciplina* d2 = *(Disciplina**)b;

    // Período 0 sempre vai pro final
    if (d1->periodo == 0 && d2->periodo != 0) return 1;
    if (d1->periodo != 0 && d2->periodo == 0) return -1;

    // Ordena por período crescente
    if (d1->periodo != d2->periodo)
        return d1->periodo - d2->periodo;

    // Prioriza obrigatórias (tipo == 1)
    return d2->tipo - d1->tipo;
}

int comparar_periodo_e_enfase(const void* a, const void* b) {
    Disciplina* d1 = *(Disciplina**)a;
    Disciplina* d2 = *(Disciplina**)b;

    // Agora: período 0 deve vir ANTES
    if (d1->periodo != d2->periodo)
        return (d1->periodo == 0) ? -1 :
               (d2->periodo == 0) ? 1 :
               d1->periodo - d2->periodo;

    // Dentro do mesmo período: peso == 0 (ênfase) vem primeiro
    int d1_enfase = (d1->peso == 0);
    int d2_enfase = (d2->peso == 0);

    return d2_enfase - d1_enfase;
}

//Função p mostrar como ficou organizado as disciplinas, mas pode apagar se n for necessário para apresentação
void imprimir_disciplinas_por_lotes(Disciplina** disciplinas, int total, int max_por_periodo) {

    int index = 0;
    int periodo_simulado = 1;

    while (index < total) {
        wprintf(L"\n===== Período Simulado %d =====\n", periodo_simulado);
        for (int i = 0; i < max_por_periodo && index < total; i++, index++) {
            Disciplina* d = disciplinas[index];

            wprintf(L"\nNome: %ls\n", d->nome);
            wprintf(L"Periodo original: %d | Carga: %d | Peso: %d | Lab: %d\n", d->periodo, d->carga, d->tipo, d->lab);
            wprintf(L"Horário: %ls\n", d->horario);
            wprintf(L"Requisitos: ");
            if (d->requisitos && d->requisitos[0]) {
                for (int j = 0; d->requisitos[j]; j++) {
                    wprintf(L"%ls ", d->requisitos[j]);
                }
            } else {
                wprintf(L"Nenhum");
            }
            wprintf(L"\n-----------------------------\n");
        }
        periodo_simulado++;
    }
}

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

   for (int i = 0; nome[i] != L'\0'; ++i) soma += value_string(towlower(nome[i]));
   return soma;
}

//função para separação do nome em partes para fazer a divisão
void name_process(Aluno aluno, int resto[]) {
    wchar_t copiaNome[60];
    wchar_t * ultimaParada; //ponteiro que guarda a posição de onde a função wcstok parou
    wchar_t * delimitadores = L" "; //ponteiro que armazena os delimitadores da função wcstok que nesse caso é somente o espaço

    int j = 0; //indice para o array de inteiros e limitador de palavras
    int soma = 0; //guardará a soma das letras 
    
    while (j < 4) {//loop para caso o nome seja pequeno e não consiga suprir os requisitos
        wcscpy(copiaNome, aluno.nome); //apesar de aluno.nome ser uma cópia iremos criar mais uma cópia por prevenção
        
        //retorna uma substring da string nome
        //recebe uma string, seus delimitadores e a última posição do ponteiro que é inicialmente NULL
        wchar_t * tk = wcstok(copiaNome, delimitadores, &ultimaParada); 

        while (tk != NULL) { //vai separar e ler cada partição, ou palavra, do nome
            if (j > 3) break;//para caso o nome da pessoa seja muito extenso
     
            if (wcslen(tk) > 3) {//caso o tamanho da palavra for <= 3 a condição irá ignorar essa palavra e vai pular para a próxima
                soma = name_sum(tk);
                resto[j] = (soma % 3);
                //wprintf(L"%d° palavra do nome: %ls, tem %ld letras e a soma das suas letras eh: %d\n", j + 1, tk, wcslen(tk), soma);
                j++;
            }
            tk = wcstok(NULL, delimitadores, &ultimaParada);  //esse NULL é para dizer para ela continuar o processo
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

    Curso* curso = (Curso*)malloc(sizeof(Curso));
    if (!curso) {
        perror("Erro ao alocar memória para o curso");
        return 1;
    }

    //inicializar o curso
    curso->nome = (wchar_t*)malloc(100 * sizeof(wchar_t));
    wcscpy(curso->nome, L"Ciência da Computação");

    //carrega os txt
    carregarDisc("disciplinas.txt", &curso->disciplinas, &curso->qtd_disciplinas);
    carregarProf("professores.txt", &curso->professores, &curso->qtd_prof);
    carregarAluno("alunos.txt", &curso->alunos, &curso->qtd_alunos);

    //dados do curso
    curso->ofertas = NULL;
    curso->qtd_ofertas = 0;
    curso->salas = NULL;
    curso->qtd_salas = 0;
    
    //indices do curso
        wprintf(L"Curso: %ls\n", curso->nome);
        wprintf(L"Disciplinas: %d\n", curso->qtd_disciplinas);
        wprintf(L"Professores: %d\n", curso->qtd_prof);
        wprintf(L"Alunos: %d\n", curso->qtd_alunos);
  
    //ofertar disciplinas
    ofertarDisc(curso);
   
    //libera memoria
    freeCurso(curso);
    free(curso);
    
    return 0;
}
