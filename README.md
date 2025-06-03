# Gerador de Oferta Acadêmica - Estrutura de Dados

Este projeto foi desenvolvido para a disciplina de Estrutura de Dados do curso de Ciência da Computação da Universidade Federal de Alagoas (UFAL). O objetivo é criar um sistema em C que gera a oferta de disciplinas para um semestre letivo, alocando professores e salas de acordo com um conjunto complexo de regras e restrições baseadas no PPC2019 do curso.

---

### ⚠️ Status do Projeto: Incompleto

O projeto encontra-se em fase de desenvolvimento e **não está totalmente funcional**. Os seguintes pontos são conhecidos e estão pendentes de implementação ou correção:

* **Erro de `Segmentation fault (core dumped)`:** Existe um erro de acesso indevido à memória que interrompe a execução do programa. A depuração está em andamento.
* **Alocação de Salas:** A lógica para alocar as salas às disciplinas ofertadas ainda não foi implementada ou precisa ser aprimorada para atender a todos os critérios.
* **Refatoração:** Partes do código, como a função `professorApto`, podem ser refatoradas para maior clareza e manutenibilidade.

---

### 🧠 A Lógica do Projeto: Regras Baseadas em Nome

Uma característica central deste projeto é que as principais regras de negócio são determinadas dinamicamente a partir dos nomes dos integrantes da equipe. Conforme o enunciado, foi escolhido o nome do aluno que vem por último em ordem alfabética: **Leandro Marcio Elias da Silva**.

As regras são calculadas da seguinte forma:

1.  **Mapeamento de Letras para Pesos:**
    > q=1, w=6, e=7, r=6, t=5, y=2, u=3, i=8, o=9, p=4, á=3, ã=4, a=2, s=5, d=8, f=7, g=4, h=1, j=4, k=7, l=8, ç=5, é=2, í=3, z=3, x=4, c=9, v=8, b=3, n=2, m=5, ó=6, õ=7, ô=6, â=1, ê=2

2.  **Cálculos e Regras Aplicadas:**
    * **Nome base para o cálculo:** `Leandro Marcio`
    * **Primeiro nome:** `Leandro` -> Soma dos pesos = $8+7+2+2+8+6+9 = 42$
    * **Próximo nome:** `Marcio` -> Soma dos pesos = $5+2+6+9+8+9 = 39$

    > **Limite de Disciplinas por Professor:**
    > (Soma do primeiro nome) % 3 = $42 \pmod 3 = 0$
    > **Resultado:** Cada professor pode lecionar no **máximo 3 disciplinas**.

    > **Solução para Disciplina sem Professor:**
    > (Soma do próximo nome) % 3 = $39 \pmod 3 = 0$
    > **Resultado:** Considerar a possibilidade de **solicitar um professor de outro instituto**.

    > **Distribuição do Professor nos Dias:**
    > (Soma do próximo nome) % 3 = $39 \pmod 3 = 0$
    > **Resultado:** Os professores devem ser alocados no **menor número de dias possível**.

    > **Ordem de Prioridade para Ofertar Disciplinas:**
    > (Soma do próximo nome) % 3 = $39 \pmod 3 = 0$
    > **Resultado:** As **disciplinas com pré-requisitos têm maior prioridade**.

---

### ✨ Funcionalidades

* **Carregamento de Dados:** Leitura de informações de alunos, professores e disciplinas a partir de arquivos de texto (`.txt`).
* **Processamento de Regras:** Aplicação das regras dinâmicas baseadas no nome do aluno para definir as restrições da oferta.
* **Verificação de Pré-requisitos:** Análise do histórico dos alunos para determinar a demanda real por cada disciplina.
* **Decisão de Oferta:** Lógica para decidir se uma disciplina deve ou não ser ofertada, com base no número de alunos interessados e em regras específicas (ex: alunos em prazo máximo de conclusão).
* **Qualificação de Professores:** Verificação da aptidão de um professor para lecionar disciplinas (especialmente as de períodos mais avançados e ênfases) com base em sua formação (mestrado/doutorado).
* **Alocação de Professores:** Atribuição de professores qualificados e disponíveis às disciplinas a serem ofertadas.
* **Ordenação de Disciplinas:** Priorização da oferta utilizando `qsort` com funções de comparação customizadas para seguir as regras definidas.

---

### 🔧 Como Compilar e Executar

**Pré-requisitos:**
* Um compilador C (GCC é recomendado).
* O ambiente de execução deve suportar a localidade `pt_BR.UTF-8` para a correta exibição de caracteres acentuados.

**Arquivos de Entrada:**
O programa necessita dos seguintes arquivos `.txt` no mesmo diretório do executável:
* `disciplinas.txt`: Contém a lista de todas as disciplinas do curso.
* `professores.txt`: Contém a lista de professores, suas formações e carga horária.
* `alunos.txt`: Contém o histórico de todos os alunos, com as disciplinas já cursadas e suas notas.
* `sala.txt`: Lista de salas e laboratórios disponíveis, com suas capacidades.

**Passos:**
1.  Clone o repositório:
    ```bash
    git clone <URL_DO_SEU_REPOSITORIO>
    cd <NOME_DO_DIRETORIO>
    ```

2.  Compile o código. O uso da flag `-Wall` é recomendado para exibir todos os avisos (warnings).
    ```bash
    gcc -o gerador_oferta main.c -Wall
    ```

3.  Execute o programa:
    ```bash
    ./gerador_oferta
    ```

---

### 🏗️ Estruturas de Dados Utilizadas

O projeto é modelado utilizando um conjunto de `structs` para representar as entidades do mundo real:

* `Disciplina`: Armazena informações como código, nome, carga horária, período, pré-requisitos e se necessita de laboratório.
* `Professor`: Contém dados do professor, como nome, formação (graduação, mestrado, doutorado), carga de trabalho e um calendário de disponibilidade.
* `Sala`: Representa uma sala ou laboratório, com seu código, capacidade e calendário de disponibilidade.
* `Aluno`: Guarda o histórico de um aluno, incluindo nome, período e as listas de disciplinas cursadas e pendentes.
* `Oferta`: Vincula uma `Disciplina` a um `Professor` e a uma `Sala` para um determinado semestre.
* `Curso`: A super-estrutura que agrega todos os ponteiros para as listas de disciplinas, professores, alunos, salas e ofertas, centralizando o controle do programa.

---

### 👥 Autores

* Erivaldo José
* Guilherme Alessander
* João Victor
* **Leandro Marcio** (Nome base para os cálculos das regras)
