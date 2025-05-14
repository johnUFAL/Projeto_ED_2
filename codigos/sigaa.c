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
    int disponibidade[6][12]; //seis dias e 12 horarios de aula
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

void liberarSala(Sala* S){

    if(!S)return;

    for(int i = 0; i < 6; i++){

        free(S->disponibilidade[i]);
    }
    free(S->disponibilidade);
    free(S->codigo);
    free(S);
}

int marcarHorario(Sala* S, int dia, int aula) {
    if (!S || dia < 0 || dia >= 6 || aula < 0 || aula >= 12) return 0;
    if (S->disponibilidade[dia][aula] == 1) return 0; // já ocupado

    S->disponibilidade[dia][aula] = 1; // marcar como ocupado
    return 1;
}

//parte para alocação

//parte para prioridades
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

//funcao para ofertar as disciplinas 
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

    wprintf(L"-> A oferta das disciplinas se dará: ");

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
Curso* carregarCurso(const char* arq_disc, const char* arq_prof, const char* arq_aluno) {
   
    FILE* file;wprintf(L"[DEBUG] Iniciando carregarCurso()\n");

    wchar_t linhas[200];
    Curso* curso = (Curso*)malloc(sizeof(Curso));
    if (!curso) return NULL;

    //entradas
    curso->alunos = NULL;
    curso->disciplinas = NULL;
    curso->nome = NULL;
    curso->salas = NULL;
    curso->ofertas = NULL;
    curso->qtd_alunos = 0;
    curso->qtd_prof = 0;

    //carregando disc
    wprintf(L"[DEBUG] Abrindo arquivo de disciplinas: %s\n", arq_disc);

    file = fopen(arq_disc, "r, ccs=UTF-8"); //win
    if (!file) {
        wprintf(L"Erro ao abrir arquivo de disciplinas\n");
        free(curso);
        return NULL;
    }

    if (file) {
        int cont = 0;
        while (fgetws(linhas, 200, file)) cont++;
        rewind(file);

        curso->disciplinas = (Disciplina**)malloc((cont+1) * sizeof(Disciplina*));
        int i = 0;
        while (fgetws(linhas, 200, file)) {
            if (wcslen(linhas) < 2) continue; //linhas vazias puladas
            Disciplina* disc = (Disciplina*)malloc(sizeof(Disciplina));
            if (!disc) {
                printf("Erro ao alocar memória para disciplina\n");
                continue;
            }
            disc->nome = NULL;
            disc->id = NULL;
            disc->horario = NULL;
            disc->requisitos = NULL;

            wchar_t* tk;
            wchar_t* contexto = NULL;

            //dados da disc
            tk = wcstok(linhas, L",", &contexto);
            while (tk) {
                if (wcsstr(tk, L"Periodo:")) {
                    disc->periodo = wcstol(wcstok(NULL, L" ", &contexto), NULL, 10);
                } else if (wcsstr(tk, L"Nome:")) {
                    disc->nome = wcsdup(wcstok(NULL, L",", &contexto));
                    if (!disc->nome) {
                        wprintf(L"Erro ao alocar nome wcsdup\n");
                    }
                } else if (wcsstr(tk, L"Id:")) {
                    wchar_t id_str[20];
                    swprintf(id_str, 20, L"%ld", wcstol(wcstok(NULL, L" ", &contexto), NULL, 10));
                    disc->id = wcsdup(id_str);
                    if (!disc->id) {
                        wprintf(L"Erro ao alocar id wcsdup\n");
                    }
                } else if (wcsstr(tk, L"Peso:")) {
                    disc->peso = wcstol(wcstok(NULL, L" ", &contexto), NULL, 10);
                } else if (wcsstr(tk, L"CH:")) {
                    disc->carga = wcstol(wcstok(NULL, L" ", &contexto), NULL, 10);
                } else if (wcsstr(tk, L"Requisito:")) {
                    wchar_t* req = wcstok(NULL, L",", &contexto);
                    if (wcscmp(req, L"0") == 0 || wcscmp(req, L"Nenhum") == 0) {
                        disc->requisitos = (wchar_t**)malloc(MAX_REQUISITOS * sizeof(wchar_t*));
                        disc->requisitos[0] = NULL;
                    } else {
                        //multiplos requisitos
                        disc->requisitos = (wchar_t**)malloc(MAX_REQUISITOS * sizeof(wchar_t*));
                        wchar_t* req_tk;
                        int j = 0;
                        req_tk = wcstok(req, L"_", &contexto);
                        while (req_tk && j < MAX_REQUISITOS) {
                            disc->requisitos[j++] = wcsdup(req_tk);
                            if (!disc->requisitos[j-1]) {
                                wprintf(L"Erro ao alocar requisito wcsdup\n");
                            }
                            req_tk = wcstok(NULL, L"_", &contexto);
                        }
                        disc->requisitos[j] = NULL;
                    }
                }
                else if (wcsstr(tk, L" Horario:")) {
                    disc->horario = wcsdup(wcstok(NULL, L",", &contexto));
                    if (!disc->horario) {
                        wprintf(L"Erro ao alocar horario wcsdup\n");
                    }
                }   
                tk = wcstok(NULL, L",", &contexto);
            }

            //definer obrigatorio=0 ou eletiva=1
            disc->tipo = (disc->periodo == 0) ? 1 : 0;
            disc->lab = 0; //TEM QUE VER ISSO AQUI

            curso->disciplinas[i++] = disc;
        }
        curso->disciplinas[i] = NULL;
        fclose(file);
    }

    //carregar prof
    wprintf(L"[DEBUG] Disciplinas carregadas. Abrindo professores: %s\n", arq_prof);

    file = fopen(arq_prof, "r, ccs=UTF-8");
    if (!file) {
        wprintf(L"Erro ao abrir arquivo de professores\n");
        free(curso);
        return NULL;
    }

    if (file) {
        int cont = 0;
        while (fgetws(linhas, 200, file)) cont++;
        rewind(file);

        curso->professores = (Professor**)malloc((cont+1) * sizeof(Professor*));

        int i = 0;
        while (fgetws(linhas, 200, file)) {
            Professor* prof = (Professor*)malloc(sizeof(Professor));
            if (!prof) {
                printf("Erro ao alocar memória para professor\n");
                continue;
            }
            wchar_t* tk;
            wchar_t* contexto = NULL;

            tk = wcstok(linhas, L",", &contexto);
            while (tk) {
                if (wcsstr(tk, L"Nome:")) {
                    prof->nome = wcsdup(wcstok(NULL, L",", &contexto));
                }
                else if (wcsstr(tk, L" Formação:")) {
                    prof->formacao = wcsdup(wcstok(NULL, L",", &contexto));
                    
                    //especializacoes
                    wchar_t* esp_tk;
                    wchar_t* esp_contexto = NULL;
                    prof->especializacao = (wchar_t**)malloc(10 * sizeof(wchar_t*));
                    int j = 0;
                    esp_tk = wcstok(prof->formacao, L",", &esp_contexto);
                    while (esp_tk && j < 10) {
                        prof->especializacao[j++] = wcsdup(esp_tk);
                        esp_tk = wcstok(NULL, L",", &esp_contexto);
                    }
                    prof->especializacao[j] = NULL;
                }
                tk = wcstok(NULL, L",", &contexto);
            }

            //disponibilidade de horsrio
            for (int d = 0; d < 6; d++) {
                for (int h = 0; h < 12; h++) {
                    prof->disponibidade[d][h] = 0;
                }
            }

            curso->professores[i++] = prof;
            curso->qtd_prof++;
        }
        curso->professores[i] = NULL;
        fclose(file);
    }

    //carregar aluno
    wprintf(L"[DEBUG] Professores carregados. Abrindo alunos: %s\n", arq_aluno);

    file = fopen(arq_aluno, "r, ccs=UTF-8");
    if (!file) {
        wprintf(L"Erro ao abrir arquivo de alunos\n");
        free(curso);
        return NULL;
    }
    if (file) {
        int cont = 0;
        while (fgetws(linhas, 10000, file)) {
            if (wcsstr(linhas, L"Nome:")) cont++;
        }
        rewind(file);

        curso->alunos = (Aluno**)malloc((cont+1) * sizeof(Aluno*));
        if (!curso->alunos) {
            printf("Erro ao alocar memória para alunos\n");
        }
        
        Aluno* aluno_atual = NULL;
        int i = 0;
        while (fgetws(linhas, 10000, file)) {
            if (wcsstr(linhas, L"Nome:")) {
                if (aluno_atual) {
                    curso->alunos[i++] = aluno_atual;
                    curso->qtd_alunos++;

                }
                
                aluno_atual = (Aluno*)malloc(sizeof(Aluno));
                if (!aluno_atual) {
                    printf("Erro ao alocar memória para aluno atual\n");
                    continue;
                }
                
                aluno_atual->nome = wcsdup(wcstok(NULL, L",", NULL));
                
                //prox perido
                fgetws(linhas, 10000, file);
                aluno_atual->periodo = wcstol(wcstok(NULL, L",", NULL), NULL, 10);
                
                //init disc vetor
                aluno_atual->disciplinas_feitas = (wchar_t**)malloc(50 * sizeof(wchar_t*));
                aluno_atual->disciplinas_falta = (wchar_t**)malloc(50 * sizeof(wchar_t*));
                int feitas_qtd = 0, falta_qtd = 0;
                
                //disc feitas
                while (fgetws(linhas, 10000, file) && wcsstr(linhas, L"Id:")) {
                    aluno_atual->disciplinas_feitas[feitas_qtd++] = wcsdup(wcstok(NULL, L",", NULL));
                    //pula o restp
                }
                aluno_atual->disciplinas_feitas[feitas_qtd] = NULL;
                aluno_atual->disciplinas_falta[falta_qtd] = NULL;
            }
        }

        //ultimo aluno
        if (aluno_atual) {
            curso->alunos[i++] = aluno_atual;
            curso->qtd_alunos++;
        }
        curso->alunos[i] = NULL;
        fclose(file);
    }
    wprintf(L"[DEBUG] Curso carregado com sucesso\n");

    return curso;
}

void liberarCurso(Curso* curso) {
    if (!curso) return;

    //libera pra disc
    if (curso->disciplinas) {
        for (int i = 0; curso->disciplinas[i] != NULL; i++) {
            free(curso->disciplinas[i]->nome);
            free(curso->disciplinas[i]->horario);

            if (curso->disciplinas[i]->requisitos) {
                for (int j = 0; curso->disciplinas[i]->requisitos[j] != NULL; j++) {
                    free(curso->disciplinas[i]->requisitos[j]);
                }
                free(curso->disciplinas[i]->requisitos);
            } 
            free(curso->disciplinas[i]);
        }
        free(curso->disciplinas);
    }

    //liberar profs
    if (curso->professores) {
        for (int i = 0; curso->professores[i] != NULL; i++) {
            free(curso->professores[i]->nome);
            free(curso->professores[i]->formacao);

            //free espec
            if (curso->professores[i]->especializacao) {
                for (int j = 0; curso->professores[i]->especializacao[j] != NULL; j++) {
                    free(curso->professores[i]->especializacao[j]);
                }
                free(curso->professores[i]->especializacao);
            }
            free(curso->professores[i]);
        }
        free(curso->professores);
    }

    //free alunos fedidos 
    if (curso->alunos) {
        for (int i = 0; curso->alunos[i] != NULL; i++) {
            free(curso->alunos[i]->nome);

            //free disc feits
            if (curso->alunos[i]->disciplinas_feitas) {
                for (int j = 0; curso->alunos[i]->disciplinas_feitas[j] != NULL; j++) {
                    free(curso->alunos[i]->disciplinas_feitas[j]);
                }
                free(curso->alunos[i]->disciplinas_feitas);
            }

            //free falta
            if (curso->alunos[i]->disciplinas_falta) {
                for (int j = 0; curso->alunos[i]->disciplinas_falta[j] != NULL; j++) {
                    free(curso->alunos[i]->disciplinas_falta[j]);
                }
                free(curso->alunos[i]->disciplinas_falta);
            }
            free(curso->alunos[i]);
        }
        free(curso->alunos);
    }

    //free salinhas 
    if (curso->salas) {
        for (int i = 0; curso->salas[i] != NULL; i++) {
            liberarSala(curso->salas[i]);
        }
        free(curso->salas);
    }

    //as ofertas
    if (curso->ofertas) {
        for (int i = 0; curso->ofertas[i] != NULL; i++) {
            free(curso->ofertas[i]->horario);
            free(curso->ofertas[i]->semestre);
            free(curso->ofertas[i]->matriculados);
            free(curso->ofertas[i]);
        }
        free(curso->ofertas);
    }
    free(curso->nome);
    free(curso);
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

    //dados do curso
    Curso* curso = carregarCurso("disciplinas.txt", "professores.txt", "alunos.txt");
    if (!curso) {
        wprintf(L"Erro ao carregar dados do curso!\n");
        return 1;
    }

<<<<<<< HEAD
    liberarSala(sala1);

    const wchar_t* nome_arquivo = L"disciplinas.txt";
    int total = 0;

    Disciplina** disciplinas = ler_disciplinas(nome_arquivo, &total);
    if (!disciplinas) {
        fwprintf(stderr, L"Erro ao carregar disciplinas do arquivo.\n");
        return 1;
    }

    if (resto[0] == 0){
        qsort(disciplinas, total, sizeof(Disciplina*), comparar_periodo_e_prereq);
        imprimir_disciplinas_por_lotes(disciplinas, total, 3);
    }

    else if (resto[0] == 1) {
        qsort(disciplinas, total, sizeof(Disciplina*), comparar_periodo_e_obrigatoriedade);
        imprimir_disciplinas_por_lotes(disciplinas, total, 2);
    }

    else if (resto[0] == 2) {
        qsort(disciplinas, total, sizeof(Disciplina*), comparar_periodo_e_enfase);
        imprimir_disciplinas_por_lotes(disciplinas, total, 1);
    }
=======
    //ofertar disciplinas
    ofertarDisc(curso);
    liberarCurso(curso);
>>>>>>> 22b6760e7501f10ace11f0d100c893094b446352
    
    return 0;
}