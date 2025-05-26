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
#define PRAZO_MAX 12 //o maximo de periodos para qualquer aluno é 12
#define MAX_DISC 3 //de acordo com o nome esse é o maximo de disciplinas
#define MAX_REQUISITOS 10 //o máximo de requisitos por disciplina

//char* armazena um nome (uma string)
//char** armazena muitas strings (um array de string)

typedef struct {
    int lab; //caso precise de laboratorio
    int carga;
    int periodo; //no caso das eletivas o periodo é 0
    int peso; //3 = computação e periodo <= 4, 2 = eletiva ou computação e periodo > 4, 1 = restante
    wchar_t* id;
    wchar_t* nome;
    wchar_t* horario;
    wchar_t** requisitos; //pode haver multiplos requisitos
} Disciplina;

typedef struct {
    int carga; //carga de trabalho maxima
    int num_disciplinas; //qtd de disciplinas matriculadas
    int disponibilidade[6][12]; //seis dias (seg-sab) e 12 horarios de aula (manha e tarde)
    wchar_t* nome;
    wchar_t* mestrado;
    wchar_t* graduacao;
    wchar_t* doutorado;
} Professor;

typedef struct {
    int eh_lab; //0 se nao, 1 se sim
    int capacidade;
    int** disponibilidade;
    wchar_t* codigo;
} Sala;

typedef struct {
    int periodo;
    int matricula;
    wchar_t* nome;
    wchar_t** disciplinas_falta;
    wchar_t** disciplinas_feitas;
} Aluno;

//o que tiver qtd provavelmente eh um contador <- comentario buxa, como assim provavelmente? tu é o programador <- "comentário chad, tem que botar pocando mesmo" by chatGPT

typedef struct {
    int qtd; 
    Sala* sala;
    wchar_t* horario;
    wchar_t* semestre;
    Professor* professor;
    Aluno** matriculados;
    Disciplina* disciplina;
} Oferta; //oferta da disciplina, com o nome do professor que irá ministrala, qtd de alunos, etc

typedef struct {
    int qtd_prof;
    int qtd_salas;
    int qtd_alunos;
    int qtd_ofertas;
    int qtd_disciplinas;  
    Sala** salas;
    wchar_t* nome;
    Aluno** alunos;
    Oferta** ofertas;
    Professor** professores;
    Disciplina** disciplinas;
} Curso; //super estrutura do curso de ciencias da computacao

//ordenação da disciplinas
int comparar_periodo_e_obrigatoriedade(const void* a, const void* b) { //const pois não queremos alterar o valor de cada disciplina, apenas ler
    Disciplina* d1 = *(Disciplina**)a;
    Disciplina* d2 = *(Disciplina**)b;

    //Ordenação entre uma obrigatória e eletiva
    //(trás) >>> (frente)
    if (d1->periodo == 0 && d2->periodo != 0) return 1; //d1 avança uma posição, ou última posição
    if (d1->periodo != 0 && d2->periodo == 0) return -1; //d1 volta uma posição, ou primeira posição

    //Ordenação entre matérias obrigatórias
    //if (d1->periodo != d2->periodo)
        //return d1->periodo - d2->periodo; //d1 pode avançar ou voltar x (d1->periodo - d2->periodo) posições 

    //Ordenação entre disciplinas eletivas por ordem alfabética
                //0 é igual, 1 d1 é maior, ou vem depois, -1 d1 é menor, ou vem antes
    
    //ordena por perido crescente pra obgt
    return d1->periodo, d2->periodo; 
}

int extrairDia(wchar_t* horario) {
    // Exemplo: se o horário = L"2M34", retornar 2
    return horario[0] - L'0'; 
}

int contarDiasAlocados(Professor* prof, Curso* curso, wchar_t* semestre_atual) {
    int diasAlocados[6] = {0};
    for (int i = 0; i < curso->qtd_ofertas; i++) {
        Oferta* oferta = curso->ofertas[i];
        if (oferta->professor == prof && wcscmp(oferta->semestre, semestre_atual) == 0) {
            // Supondo que oferta->horario tem um formato "DHH:MM-HH:MM" onde D é dia (0..5)
            // ou algo que permita identificar o dia
            int dia = extrairDia(oferta->horario);
            diasAlocados[dia] = 1;
        }
    }
    int soma = 0;
    for (int d = 0; d < 6; d++) soma += diasAlocados[d];
    return soma;
}

int marcarHorario(Sala* S, int dia, int aula) {
    if (!S || dia < 0 || dia >= 6 || aula < 0 || aula >= 12) return 0;
    if (S->disponibilidade[dia][aula] == 1) return 0; // já ocupado

    S->disponibilidade[dia][aula] = 1; // marcar como ocupado
    return 1;
}

//essa função tem o objetivo de encontrar um bloco contínuo de horários disponíveis na agenda de um professor
//é aqui que vamos garantir que ele seja alocado no menor numero de dias possiveis
void buscarBlocoHorarioDisponivel(Professor* prof, int carga, int* dia_out, int* hora_out) {
    for (int dia = 0; dia < 6; dia++) {
        int count = 0;
        for (int hora = 0; hora < 12; hora++) {
            if (prof->disponibilidade[dia][hora] == 0) {
                count++;
                if (count == carga) {
                    // bloco encontrado, hora inicial = hora - carga + 1
                    *dia_out = dia;
                    *hora_out = hora - carga + 1;
                    return;
                }
            } else {
                count = 0; // reset quando encontrar ocupado
            }
        }
    }
    // se não achou nenhum bloco
    *dia_out = -1;
    *hora_out = -1; 
}

//aqui vai marcar o horario do professor, para se por exemplo ele ja tem aula de 7h as 10h nao pegar esse horario em tal dia
void marcarHorarioProfessor(Professor* prof, int dia, int hora_inicio, int carga) {
    for (int i = 0; i < carga; i++) {
        prof->disponibilidade[dia][hora_inicio + i] = 1; // marca como ocupado
    }
}

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

//decisao das ofertas de disciplinas, ou seja, será analisado se uma disciplina está apta para oferecimento 
int decisaoOfertaDisc(Disciplina* disciplina, Aluno** alunos, int num_alunos, int periodoMax) {
    
    //variáveis para contabilizar quantos alunos podem cursar tal disciplina
    int interessados = 0;  //querem fazer a disciplina
    int no_prazo = 0; //estao no prazo limite do curso

    for (int i = 0; i < num_alunos; i++) {
        int feita = 0; //se já pagou a disciplina
        for (int j = 0; alunos[i]->disciplinas_feitas[j] != NULL; j++) { //vai analisar todas as disciplinas pagas pelo aluno i
            if (wcscmp(alunos[i]->disciplinas_feitas[j], disciplina->id) == 0) { //se der igual quer dizer que este aluno já pagou essa matéria
                feita = 1;
                break;
            }
        }

        if (feita) continue; //pula para o próximo aluno

        //pre requisito, caso o aluno ainda não tenha pagado essa disciplina
        int matricular = 1; //vai ser o resultado final após a checagem dos x pre requisitos, 1 - pode se matricular, 0 - não pode se matricular 
        int permitido = 0; //vai dizer se ele cumpre com todos os pre requisitos 
        if (disciplina->requisitos != NULL && disciplina->requisitos[0] != NULL && wcscmp(disciplina->requisitos[0], L"NULL") != 0) {
            for (int k = 0; disciplina->requisitos[k] != NULL; k++) { //vai caminhando por todos os pre requisitos da disciplina                
                for (int l = 0; alunos[i]->disciplinas_feitas[l] != NULL; l++) { //vai caminhando por todas as disciplinas pagas pelo aluno, para ver se ele pode pagar esta disciplina
                    if (wcscmp(alunos[i]->disciplinas_feitas[l], disciplina->requisitos[k]) == 0) {
                        permitido = 1;
                        break;
                    }
                }

                if (!permitido) { //para caso o loop das disciplinas que ele tenha pago tenha sido terminado e não tenha sido
                    //encontrado a correspondente com um dos pre requisitos
                    matricular = 0; //não poderá se matricular
                    break;
                }
            }
        }

        if (matricular) { //pode se matricular
            interessados++;
            if (alunos[i]->periodo == periodoMax) { //caso o aluno esteja no periodo máximo do curso e esteja matriculado na disciplina
                //é obrigátório que esta seja ofertada
                no_prazo++;
            }
        }
    }

    //não ofertamento
    if (interessados == 0) return 0;
    if(interessados < 10 && no_prazo == 0) {
        wprintf(L"Sem alunos minimos ou aluno no prazo máximo do curso! Não será possivel oferta-la\n");
        return 0;
    }

    return 1; // verdadeiro
}

int professorApto(Disciplina* disciplina, Professor* professores) { //função que retorna 1 ou 0, cujo objetivo é dizer se o professor é apto a partir do seu doutorado ou mestrado
    if (wcsstr(disciplina->nome, L"Direito") && (wcsstr(professores->doutorado, L"Direito") || wcsstr(professores->mestrado, L"Direito"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Programacao") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Algoritmos") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Matematica") || wcsstr(professores->mestrado, L"Matematica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Teoria") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Matematica") || wcsstr(professores->mestrado, L"Matematica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Operacionais") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Compiladores") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Artificial") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica") || wcsstr(professores->doutorado, L"Conhecimento") || wcsstr(professores->mestrado, L"Conhecimento"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Grafica") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Desenvolvimento") && (wcsstr(professores->doutorado, L"Engenharia") || wcsstr(professores->mestrado, L"Engenharia") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Metodologias") && (wcsstr(professores->doutorado, L"Administracao") || wcsstr(professores->mestrado, L"Administracao") || wcsstr(professores->doutorado, L"Producao") || wcsstr(professores->mestrado, L"Producao"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Apredizagem") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Computacional") || wcsstr(professores->mestrado, L"Computacional"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Digitais") && (wcsstr(professores->doutorado, L"Eletrica") || wcsstr(professores->mestrado, L"Eletrica") || wcsstr(professores->doutorado, L"Engenharia da Computacao") || wcsstr(professores->mestrado, L"Engenharia da Computacao"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Distribuidos") && (wcsstr(professores->doutorado, L"Computacao") || wcsstr(professores->mestrado, L"Computacao") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Interacao") && (wcsstr(professores->doutorado, L"Computacao") || wcsstr(professores->mestrado, L"Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Evolucionaria") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Conhecimento") || wcsstr(professores->mestrado, L"Conhecimento"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Embarcados") && (wcsstr(professores->doutorado, L"Eletrica") || wcsstr(professores->mestrado, L"Eletrica") || wcsstr(professores->doutorado, L"Engenharia da Computacao") || wcsstr(professores->mestrado, L"Engenharia da Computacao"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Gerencia") && (wcsstr(professores->doutorado, L"Administracao") || wcsstr(professores->mestrado, L"Administracao") || wcsstr(professores->doutorado, L"Producao") || wcsstr(professores->mestrado, L"Producao"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Conceitos") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Dados") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Matematica") || wcsstr(professores->mestrado, L"Matematica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Microcontroladores") && (wcsstr(professores->doutorado, L"Eletrica") || wcsstr(professores->mestrado, L"Eletrica") || wcsstr(professores->doutorado, L"Engenharia da Computacao") || wcsstr(professores->mestrado, L"Engenharia da Computacao"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Seguranca") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Sistemas") || wcsstr(professores->mestrado, L"Sistemas"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Calculo") && (wcsstr(professores->doutorado, L"Matematica") || wcsstr(professores->mestrado, L"Matematica") || wcsstr(professores->doutorado, L"Fisica") || wcsstr(professores->mestrado, L"Fisica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Neurais") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Conhecimento") || wcsstr(professores->mestrado, L"Conhecimento") ||  wcsstr(professores->doutorado, L"Matematica") || wcsstr(professores->mestrado, L"Matematica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Processamento") && (wcsstr(professores->doutorado, L"Eletrica") || wcsstr(professores->mestrado, L"Eletrica") || wcsstr(professores->doutorado, L"Computacao") || wcsstr(professores->mestrado, L"Computacao") || wcsstr(professores->doutorado, L"Informatica") || wcsstr(professores->mestrado, L"Informatica"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Visao") && (wcsstr(professores->doutorado, L"Ciencia da Computacao") || wcsstr(professores->mestrado, L"Ciencia da Computacao") || wcsstr(professores->doutorado, L"Conhecimento") || wcsstr(professores->mestrado, L"Conhecimento"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"ACE") && (wcsstr(professores->doutorado, L"Administracao") || wcsstr(professores->mestrado, L"Administracao") || wcsstr(professores->doutorado, L"Producao") || wcsstr(professores->mestrado, L"Producao"))) {
        return 1;
    } else if (wcsstr(disciplina->nome, L"Direito") && (wcsstr(professores->doutorado, L"Direito") || 
               wcsstr(professores->mestrado, L"Direito") || wcsstr(professores->graduacao, L"Direito"))) {
        return 1;
    } else return 0;
}                                   

//adaptado para ter limite maximo de 1 disciplina
//achar prof qualificado
Professor** buscarProfQualif(Professor** professores, int num_prof, Disciplina* disciplina, 
    int* prof_achados, const wchar_t* semestre_atual, Curso* curso) {
//buscarProfQualif(curso->professores, curso->qtd_prof, disc, &prof_encontrado, semestre_atual, curso)

    //alocacao de memoria pra prof qualificado 
    Professor** qualificados = malloc(num_prof * sizeof(Professor*));
    *prof_achados = 0;

    for (int i = 0; i < num_prof; i++) {
    int tem_disciplina = 0;

        //vai ver se o prof ja tem disciplina nesse semestre
        for (int o = 0; o < curso->qtd_ofertas; o++) {
            Oferta* oferta = curso->ofertas[o];
            if ((wcscmp(oferta->professor->nome, professores[i]->nome) == 0) &&
            (wcscmp(oferta->semestre, semestre_atual) == 0)) {
            tem_disciplina = 1;
            break;
            }
        }

        if (tem_disciplina) continue;

        //os criterios de qualificações para atender a disciplina
        if (disciplina->periodo <= 4) {
            qualificados[(*prof_achados)++] = professores[i];
        } else {
            if (professorApto(disciplina, professores[i])) {
                qualificados[(*prof_achados)++] = professores[i];
                }
            }
    }
    return qualificados;
}

//compara para o qsort
int comparaPesoPerido(const void* a, const void* b) {
    Disciplina* d1 = *(Disciplina**)a;
    Disciplina* d2 = *(Disciplina**)b;
    
    //ordena peso decres
    if (d1->peso != d2->peso) {
        return d2->peso - d1->peso;
    }
    
    //se peso = ordena periodo
    return d1->periodo - d2->periodo;
}

//funcao para ofertar as disciplinas com professor (funcao principal)
void ofertarDisc(Curso* curso, const wchar_t* semestre_atual) {
    //ordena por peso e periodo
    qsort(curso->disciplinas, curso->qtd_disciplinas, sizeof(Disciplina*), comparar_periodo_e_obrigatoriedade);   
    
    //so para checar!!!
    wprintf(L"\n=== Disciplinas ordenadas ===\n"); //printação das disciplinas ordenadas
    for (int i = 0; i < curso->qtd_disciplinas; i++) {
        Disciplina* d = curso->disciplinas[i];
        wprintf(L"%d. %ls, Periodo: (%d)\n", i+1, d->nome, d->periodo);
    }

    //controle de profs ja alocados na oferta
    int* prof_alocados = calloc(curso->qtd_prof, sizeof(int)); //ponteiro para o numero total de professores

    for (int i = 0; i < curso->qtd_disciplinas; i++) { //vai caminhar por todas as disciplinas
        Disciplina* disc = curso->disciplinas[i]; //ponteiro para uma das disciplinas do curso
        int pode_ofertar = decisaoOfertaDisc(disc, curso->alunos, curso->qtd_alunos, PRAZO_MAX); //1 - pode ofertar, 0 - não pode

        if (pode_ofertar) {
            int prof_encontrado = 0;
            Professor** quali = buscarProfQualif(curso->professores, curso->qtd_prof, 
                                                 disc, &prof_encontrado, semestre_atual, curso);

            if (quali != NULL && quali[0] != NULL) { //verificação para saber se houve algum professor qualificado para ministrar a disciplina

                for (int i = 0; i < prof_encontrado; ++i) { //vai checar todos os professores aptos

                }

                        //primeiro prof n alocado ainda
                        /*Professor* prof_selecionado = NULL;
                        for (int p = 0; p < prof_encontrado; p++) {
                            if (!prof_alocados[p]) {
                                prof_selecionado = quali[p];
                                prof_alocados[p] = 1;
                                break;
                            }
                        }*/
                
                        /*if (prof_selecionado) {
                        wprintf(L"\n--------------------------------------------\n");
                        wprintf(L"\n----- Professor alocado -----\n");
                        wprintf(L"%ls: %ls \n", prof_selecionado->nome, disc->nome);
                        
                        //nova oferta
                        Oferta* nova_oferta = calloc(1, sizeof(Oferta)); 
                        if (!nova_oferta) {
                            perror("Erro ao alocar nova_oferta");
                            free(prof_alocados); 
                            return; 
                        }

                        nova_oferta->disciplina = disc;
                        nova_oferta->professor = prof_selecionado;
                        nova_oferta->semestre = wcsdup(semestre_atual);
                        if (nova_oferta->semestre == NULL && semestre_atual != NULL) {
                            perror("Erro ao duplicar semestre para nova_oferta");
                            free(nova_oferta); 
                            return; 
                        }

                        curso->ofertas[curso->qtd_ofertas++] = nova_oferta;
                        }*/
            } else {
                wprintf(L"\n--------------------------------------------\n");
                wprintf(L"\nSem professor adequado para: %ls\n", disc->nome);
                wprintf(L"\n--------------------------------------------\n");
                wprintf(L"\nConsidere solicitar professor de outro instituto para: %ls\n", disc->nome);

                /*if (disc->peso >= 2) {
                    wprintf(L"\n--------------------------------------------\n");
                    wprintf(L"\nSem professor adequado para: %ls\n", disc->nome);
                } else {
                    wprintf(L"\n--------------------------------------------\n");
                    wprintf(L"\nConsidere solicitar professor de outro instituto para: %ls\n", disc->nome);
                }*/
            }

            free(quali);

        } else {
            printf("Não eres posible ofertar esta materia: %ls!\n", disc->nome);
        }
    }

    free(prof_alocados);
}

//parte para estrategias de ofertas e etc
void Situacao (int resto[], Aluno* aluno) {//essa função descreve os critérios estabelecidos pela professora{
    wprintf(L"=============================CRITÉRIOS=============================\n");
    
    wprintf(L"-> Máximo de disciplinas do professor por semestre será de ");
    
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
        case 0: wprintf(L"Os professores devem ser alocados no menor números de dias possíveis\n");
            break;
        case 1: wprintf(L"Os professores devem ser alocados no menor números de dias possíveis\n");    
            break;
        case 2: wprintf(L"Os professores devem ser alocados de modo a ir ao IC todos os dias\n");
            break;
        default: wprintf(L"#\nERRO! Valor fora do intervalo esperado!\n");
    }

    wprintf(L"-> A oferta das disciplinas se dará: ");

    switch (resto[3]) { 
        case 0: wprintf(L"As disciplinas com pré requisitos têm maior prioridade\n");
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

    wchar_t linha[200];
    *cont = 0;
    
    //conta linhas pra alocar
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Periodo:")) (*cont)++;
    }
    rewind(file);

    *disciplinas = (Disciplina**)malloc((*cont + 1) * sizeof(Disciplina*));
    if (!*disciplinas) {
        perror("Erro ao alocar disciplinas");
        fclose(file);
        exit(1);
    }

    int i = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Periodo:")) {
            Disciplina* d = (Disciplina*)malloc(sizeof(Disciplina));
            
            if (!d) {
                perror("Erro ao alocar disciplina");
                fclose(file);
                exit(1);
            }

            d->nome = NULL;
            d->id = NULL;
            d->horario = NULL;
            d->requisitos = NULL;

            d->nome = (wchar_t*)malloc(100 * sizeof(wchar_t));
            d->id = (wchar_t*)malloc(10 * sizeof(wchar_t));
            d->horario = (wchar_t*)malloc(10 * sizeof(wchar_t));
            d->requisitos = (wchar_t**)malloc(MAX_REQUISITOS * sizeof(wchar_t*));

            /*Periodo: 2, Nome: Banco de dados, Id: COMP365, CH: 72, Requisito: NULL, Horario: 24T34
            tokens exemplo
            Periodo: 2
             Nome: Banco de dados 
             Id: COMP365
             CH: 72
             Requisito: NULL
             Horario: 24T34
            */

            wchar_t* svptr;
            wchar_t* tk = wcstok(linha, L",", &svptr);
            while (tk != NULL) {
                if (wcsstr(tk, L"Periodo:")) {
                    swscanf(tk, L"Periodo: %d", &d->periodo);
                    if (d->periodo == 0) {
                        //eletivas peoso 2
                        d->peso = 2;
                    } else if (d->periodo <= 4 && wcsstr(d->nome, L"Programacao") != NULL) {
                        //ate o 4 perido obrigatorias de programacao peso 3
                        d->peso = 3;
                    } else if (d->periodo > 4 && wcsstr(d->nome, L"Programacao") != NULL) {
                        //obrigatoria de programacao apos 4 perido peso 2
                        d->peso = 2;
                    } else {
                        //demais peso 4
                        d->peso = 1;
                    }
                } else if (wcsstr(tk, L" Nome:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(d->nome, play);
                } else if (wcsstr(tk, L" Id:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(d->id, play);
                } else if (wcsstr(tk, L" CH:")) {
                    swscanf(tk, L" CH: %d", &d->carga);
                } else if (wcsstr(tk, L" Requisito:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;

                    if (wcscmp(play, L"NULL") != 0) {
                        wchar_t *ultimaPosicao;
                        wchar_t *token = wcstok(play, L"_", &ultimaPosicao);
                        int r = 0;
                        while (token != NULL && r < MAX_REQUISITOS-1) {
                            d->requisitos[r++] = wcsdup(token);
                            token = wcstok(NULL, L"_", &ultimaPosicao);
                        }
                        d->requisitos[r] = NULL;
                    } else {
                        d->requisitos[0] = NULL; 
                    }
                } else if (wcsstr(tk, L" Horario:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(d->horario, play);
                }
                tk = wcstok(NULL, L",", &svptr);
            }
            d->lab = (wcsstr(d->nome, L"Programacao") != NULL) ? 1 : 0; 
            (*disciplinas)[i++] = d;
        }
    }
    (*disciplinas)[i] = NULL;
    fclose(file);
}

void carregarProf(const char* nome_arq, Professor*** professores, int* cont) {
    FILE* file = fopen(nome_arq, "r, ccs=UTF-8");
    if (!file) {
        perror("Erro ao abrir arquivo de professores");
        exit(1);
    }

    wchar_t linha[300]; //estimativa do máximo de caracteres por linha
    *cont = 0; //contador de linhas
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) (*cont)++;
    }

    rewind(file); //retorne ao macaco, digo, retorne ao inicio

    *professores = (Professor**)malloc(*cont * sizeof(Professor*)); //vai alocar memória para o ponteiro de ponteiro, criando nossa matriz

    if (!*professores) {
        perror("Erro ao alocar professores");
        fclose(file);
        exit(1);
    }

    int i = 0;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) {
            Professor* p = (Professor*)malloc(sizeof(Professor)); //ponteiro auxiliar
            
            //foi feita uma estimativa para otimizar o uso de memória
            p->nome = (wchar_t*)malloc(55 * sizeof(wchar_t));
            p->graduacao = (wchar_t*)malloc(35 * sizeof(wchar_t));
            p->mestrado = (wchar_t*)malloc(40 * sizeof(wchar_t));
            p->doutorado = (wchar_t*)malloc(50 * sizeof(wchar_t));
            
            //preechendo toda a matriz com 1, ou seja, disponivel
            for (int d = 0; d < 6; d++) {
                for (int h = 0; h < 12; h++) {
                    p->disponibilidade[d][h] = 1;
                }
            }
            
            wchar_t* svptr;
            wchar_t* tk = wcstok(linha, L",", &svptr); //guarda a ultima posicao do token, necessário para sistemas linux
            //Nome: Leonardo Viana Pereira, CH: 40, Graduação: Fisica, Mestrado: Fisica, Doutorado: Fisica
            while (tk != NULL) {
                if (wcsstr(tk, L"Nome:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1; 
                    while (*play == L' ') play++;
                    wcscpy(p->nome, play);
                } else if (wcsstr(tk, L" CH:")) {
                    swscanf(tk, L" CH: %d", &p->carga);
                } else if (wcsstr(tk, L" Graduação:")) {
                    wchar_t* play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(p->graduacao, play);
                    
                    /*wchar_t* esp_saveptr;
                    wchar_t* esp_token = wcstok(p->graduacao, L",", &esp_saveptr);
                    int esp_count = 0;
                    while (esp_token != NULL && esp_count < 10) {
                        p->especializacao[esp_count] = (wchar_t*)malloc(100 * sizeof(wchar_t));
                        wcscpy(p->especializacao[esp_count], esp_token);
                        esp_count++;
                        esp_token = wcstok(NULL, L",", &esp_saveptr);
                    }
                    p->especializacao[esp_count] = NULL;*/

                } else if (wcsstr(tk, L" Mestrado:")) {
                    wchar_t *play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(p->mestrado, play);
                }
                else if (wcsstr(tk, L" Doutorado:")) {
                    wchar_t *play = wcsstr(tk, L":") + 1;
                    while (*play == L' ') play++;
                    wcscpy(p->doutorado, play);
                }

                tk = wcstok(NULL, L",", &svptr);
            }

            p->num_disciplinas = 0;
            (*professores)[i++] = p;
        }
    }

    (*professores)[i] = NULL;

    fclose(file);
}

void carregarAluno(const char* nome_arq, Aluno*** alunos, int* cont) {
    FILE* file = fopen(nome_arq, "r, ccs=UTF-8");
    if (!file) {
        perror("Erro ao abrir arquivo de alunos");
        exit(1);
    }

    wchar_t linha[200];
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

    int i = 0, j = 0, k = 0;
    Aluno* atual_al = NULL;
    while (fgetws(linha, sizeof(linha)/sizeof(wchar_t), file)) {
        if (wcsstr(linha, L"Nome:")) {
            if (atual_al != NULL) {
                (*alunos)[i++] = atual_al;
            }
            
            atual_al = (Aluno*)malloc(sizeof(Aluno));
            atual_al->nome = (wchar_t*)malloc(100 * sizeof(wchar_t));
            atual_al->disciplinas_feitas = (wchar_t**)malloc(50 * sizeof(wchar_t*)); //50 é a estimativa da quantidade de disciplinas
            atual_al->disciplinas_falta = (wchar_t**)malloc(50 * sizeof(wchar_t*)); //ou seja, um ponteiro de ponteiro para o 1° de 50 ponteiros wchar
            
            /*Nome: Ana Beatriz Santos Lima, Periodo: 6
            Id: COMP387, Nome: Nocoes de Direito, Nota: 9.8
            Id: COMP401, Nome: Ciencia de Dados, Nota: 8.0*/

            wchar_t* svptr; //guarda a ultima posicao do token, necessário para sistemas linux
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
            
            atual_al->disciplinas_feitas[0] = NULL; //iniciando para o novo aluno
            atual_al->disciplinas_falta[0] = NULL;
            j = 0; //indice que ajudará na distribuição das disciplinas pagas
            k = 0; //indice para as disciplinas não pagas, ou perdidas
        } 
        else if (wcsstr(linha, L"Id:")) {
            wchar_t id[20], nome[65];
            float nota;
            
            swscanf(linha, L"Id: %19[^,], Nome: %64[^,], Nota: %f", id, nome, &nota);
            
            if (nota >= 7.0) {
                while (atual_al->disciplinas_feitas[j] != NULL) j++; //caminha até chegar ao final do ponteiro e a parti daí vai ser anexado um id
                atual_al->disciplinas_feitas[j] = (wchar_t*)malloc(20 * sizeof(wchar_t));
                wcscpy(atual_al->disciplinas_feitas[j], id);
                atual_al->disciplinas_feitas[j+1] = NULL; //ajuda a finalizar a leitura 
            } else {
                while (atual_al->disciplinas_falta[k] != NULL) k++;
                atual_al->disciplinas_falta[k] = (wchar_t*)malloc(20 * sizeof(wchar_t));
                wcscpy(atual_al->disciplinas_falta[k], id);
                atual_al->disciplinas_falta[k+1] = NULL; //ajuda a finalizar a leitura 
            }
        }
    }
    
    if (atual_al != NULL) {
        (*alunos)[i++] = atual_al;
        (*alunos)[i] = NULL;
    }
    
    fclose(file);
}

void freeDisciplinas(Disciplina* d) {
    if (!d) return;

    if (d->nome) free(d->nome);
    if (d->id) free(d->id);
    if (d->horario) free(d->horario);

    if (d->requisitos) {
        // não precisa verificar d->requisitos[0] != NULL antes do loop
        for (int i = 0; d->requisitos[i] != NULL; i++) {
            free(d->requisitos[i]);
        }
        free(d->requisitos);
    }

    free(d);
}


void freeProf(Professor* p) {
    if (!p) return;
    if (p->nome) free(p->nome);
    if (p->graduacao) free(p->graduacao);
    if (p->mestrado) free(p->mestrado);
    if (p->doutorado) free(p->doutorado);
    free(p);
}

void freeAluno(Aluno* a) {
    if (!a) return;
    if (a->nome) free(a->nome);

    if (a->disciplinas_feitas) {
        for (int i = 0; a->disciplinas_feitas[i] != NULL; i++) {
            free(a->disciplinas_feitas[i]);
        }
        free(a->disciplinas_feitas);
    }

    if (a->disciplinas_falta) {
        for (int i = 0; a->disciplinas_falta[i] != NULL; i++) {
            free(a->disciplinas_falta[i]);
        }
        free(a->disciplinas_falta);
    }

    free(a);
}


void freeSala(Sala* s) {
    if (!s) return;

    if (s->codigo) {
        free(s->codigo);
    }

    if (s->disponibilidade) {
        for (int i = 0; i < 6; i++) {
            if (s->disponibilidade[i]) {
                free(s->disponibilidade[i]);
            }
        }
        free(s->disponibilidade);
    }

    free(s);
}

void freeOferta(Oferta* o) {
    if (!o) return;
    if (o->horario) free(o->horario);
    if (o->semestre) free(o->semestre);

    // Só libera o vetor de ponteiros, não os alunos
    if (o->matriculados) free(o->matriculados);

    free(o);
}

void freeCurso(Curso* curso) {
    /*typedef struct {
    int qtd_prof;
    int qtd_salas;
    int qtd_alunos;
    int qtd_ofertas;
    int qtd_disciplinas;  
    Sala** salas;
    wchar_t* nome;
    Aluno** alunos;
    Oferta** ofertas;
    Professor** professores;
    Disciplina** disciplinas;
} Curso;*/

    if (curso == NULL) printf("Não há nada para se liberar!\n"); return;

    //free sala
    int i = 0;
    while (curso->salas[i] != NULL) {
        free(curso->salas[i]);
        i++;
    }
    free(curso->salas);

    //free nome do curso
    free(curso->nome);

    //free alunos
    i = 0;
    while (curso->alunos[i] != NULL) {
        free(curso->alunos[i]);
        i++;
    }
    free(curso->alunos);

    //free ofertas
    i = 0;
    while (curso->ofertas[i] != NULL) {
        free(curso->ofertas[i]);
        i++;
    }
    free(curso->ofertas);

    //free professores
    i = 0;
    while (curso->professores[i] != NULL) {
        free(curso->professores[i]);
        i++;
    }
    free(curso->professores);

    //free disciplinas
    i = 0;
    while (curso->disciplinas[i] != NULL) {
        free(curso->disciplinas[i]);
        i++;
    }
    free(curso->disciplinas);

    return;
}


Sala** lerSalasDeArquivo(const char* nomeArquivo, int* qtd_salas) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        wprintf(L"Erro ao abrir o arquivo de salas.\n");
        return NULL;
    }

    Sala** salas = NULL;
    int capacidade, eh_lab;
    wchar_t codigo[100];
    int count = 0;

    while (fwscanf(arquivo, L"%99ls %d %d", codigo, &capacidade, &eh_lab) == 3) {
        Sala* novaSala = criarSala(codigo, capacidade, eh_lab);
        if (!novaSala) {
            wprintf(L"Erro ao criar sala.\n");
            // Libera as salas já alocadas
            for (int i = 0; i < count; i++) {
                freeSala(salas[i]);
            }
            free(salas);
            fclose(arquivo);
            return NULL;
        }

        Sala** temp = realloc(salas, (count + 1) * sizeof(Sala*));
        if (!temp) {
            wprintf(L"Erro ao realocar vetor de salas.\n");
            freeSala(novaSala);
            for (int i = 0; i < count; i++) {
                freeSala(salas[i]);
            }
            free(salas);
            fclose(arquivo);
            return NULL;
        }

        salas = temp;
        salas[count++] = novaSala;
    }

    fclose(arquivo);
    *qtd_salas = count;
    return salas;
}

//resumo do funcionamento da função imprimirSalasComOfertas
//percorre todas as salas do curso e para cada sala ela procura entre as ofertas de disciplinas quais estão alocadas naquela sala
//Imprime o nome da disciplina e outras coisas referentes a ela
void imprimirSalasComOfertas(Curso* curso) {
    wprintf(L"\n====== Salas com disciplinas alocadas ======\n");

    for (int i = 0; i < curso->qtd_salas; i++) {
        Sala* sala = curso->salas[i];
        wprintf(L"\nSala %ls | Capacidade: %d | Tipo: %ls\n",
                sala->codigo,
                sala->capacidade,
                sala->eh_lab ? L"Laboratório" : L"Sala comum");

        int encontrou = 0;

        for (int j = 0; j < curso->qtd_ofertas; j++) {
            Oferta* oferta = curso->ofertas[j];

            if (oferta->sala && wcscmp(oferta->sala->codigo, sala->codigo) == 0) {
                encontrou = 1;
                wprintf(L"  • %ls (%ls) - Horário: %ls\n",
                        oferta->disciplina->nome,
                        oferta->disciplina->id,
                        oferta->disciplina->horario);
            }
        }

        if (!encontrou) {
            wprintf(L"  Nenhuma disciplina alocada nesta sala.\n");
        }
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
/*Disciplina** ler_disciplinas(const char* nome_arquivo, int* total) {
    setlocale(LC_ALL, ""); // Garante suporte a UTF-8 no terminal
    FILE* arquivo = fopen(nome_arquivo, "r");
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
        nova->peso = wcstol(buffer, NULL, 10);

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
} */

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

int comparar_periodo_e_enfase(const void* a, const void* b) {
    Disciplina* d1 = *(Disciplina**)a;
    Disciplina* d2 = *(Disciplina**)b;

    // Disciplinas de período 0 (eletivas/ênfase) vêm primeiro
    if (d1->periodo == 0 && d2->periodo != 0) return -1;
    if (d1->periodo != 0 && d2->periodo == 0) return 1;

    // Para disciplinas do mesmo tipo (ambas ênfase ou ambas obrigatórias)
    // Ordena por período crescente
    if (d1->periodo != d2->periodo)
        return d1->periodo - d2->periodo;

    // Se forem ambas eletivas (período 0), pode usar outro critério se necessário
    // Por exemplo, ordenar alfabeticamente:
    return wcscmp(d1->nome, d2->nome);
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
            wprintf(L"Periodo original: %d | Carga: %d | Peso: %d | Lab: %d\n", d->periodo, d->carga, d->peso, d->lab);
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

int main() {
    // Ativa suporte a caracteres Unicode no terminal
    setlocale(LC_ALL, "");
    fwide(stdout, 1); // Garante modo wide para wprintf

    Aluno aluno = {.nome = L"Leandro Marcio Elias da Silva"};
    int resto[MAXR]; 

    name_process(aluno, resto);
    Situacao(resto, &aluno);

    // Aloca e inicializa a estrutura Curso
    Curso* curso = malloc(sizeof(Curso));
    if (!curso) {
        perror("Erro ao alocar memória para o curso");
        return 1;
    }

    curso->nome = malloc(30 * sizeof(wchar_t));
    wcscpy(curso->nome, L"Ciência da Computação");

    // Carrega dados dos arquivos
    carregarDisc("disciplinas.txt", &curso->disciplinas, &curso->qtd_disciplinas); //Disciplina** disciplinas
    carregarProf("professores.txt", &curso->professores, &curso->qtd_prof); //Professor** professores
    carregarAluno("alunos.txt", &curso->alunos, &curso->qtd_alunos); //Aluno** aluno
    
    curso->salas = lerSalasDeArquivo("sala.txt", &curso->qtd_salas);
        if (curso->salas == NULL && curso->qtd_salas != 0) { 
            wprintf(L"Falha ao carregar salas.\n");
        }


    //so por seguranca
    if (curso->qtd_disciplinas == 0 || curso->qtd_prof == 0 || curso->qtd_alunos == 0) {
        wprintf(L"Erro: Nenhum dado foi carregado corretamente!\n");
        freeCurso(curso);
        free(curso);
        return 1;
    }

    // Inicializa ponteiros e contadores
    curso->ofertas = (Oferta**)malloc(curso->qtd_disciplinas * sizeof(Oferta*)); // Garantir espaço
    curso->qtd_ofertas = 0;

    //curso->salas = NULL;
   // curso->qtd_salas = 0;

    // Informações do curso
    wprintf(L"\nCurso: %ls\n", curso->nome);
    wprintf(L"Quantidade de disciplinas: %d\n", curso->qtd_disciplinas);
    wprintf(L"Quantidade de professores: %d\n", curso->qtd_prof);
    wprintf(L"Quantidade de alunos: %d\n", curso->qtd_alunos);

    // Processa oferta de disciplinas
    ofertarDisc(curso, L"2025.1");
    imprimirSalasComOfertas(curso);
    

    // Liberação de memória
    freeCurso(curso);  // Esta função deve liberar tudo que estiver dentro de `curso`
    free(curso);

    return 0;
}
