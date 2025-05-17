#include <stdio.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>

int main ()
{
    FILE *file = fopen("disciplinas.txt", "r, css=UTF-8");
    wchar_t nome[60], id[12], requisito[100], horario[12];
    int peso, ch, periodo;

    if (!file)
    {
        printf("Deu ruim\n");
        return 1;
    }

    int i = 0;

    /*Periodo: 0, Nome: Sistemas Embarcados, Id: COMP398, Peso: 0, CH: 72, Requisito: Nenhum, Horario: 35T56
    while ((fwscanf(file, L"Periodo: %d, Nome: %59l[^,], Id: %11l[^,], Peso: %d, CH: %d, Requisito: %99l[^,], Horario: %11l[^\n]\n", &periodo, nome, id, &peso, &ch, requisito, horario)) != EOF)
    {
        //wprintf(L"Periodo: %d, Nome: %ls, Id: %ls, Peso: %d, CH: %d, Requisito: %ls, Horario: %ls\n", periodo, nome, id, peso, ch, requisito, horario);
        i++;
    }*/

    //Periodo: 0, Nome: Sistemas Embarcados, Id: COMP398, Peso: 0, CH: 72, Requisito: Nenhum, Horario: 35T56
    while ((fwscanf(file, L"Periodo: %d, Nome: %s[^,], Id: %l[^,], Peso: %d, CH: %d, Requisito: %l[^,], Horario: %l[^\n]\n")) != EOF)
    {
        i++;
    }

    wprintf(L"\nQtd de disciplinas: %d\n", i);

    return 0;
}