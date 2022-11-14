#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Поскольку параметры файла не известны, будем считать, что строка состоит из любого символа UTF-8, кроме
// основных не буквенных символов из ASCII. Такая небольшая фильтрация
#define SIMBOL(x) ((x >= 0x00) && (x < 0x41)) || ((x >= 0x5b) && (x < 0x61)) || ((x >= 0x7b) && (x <= 0x7F))

#define NOT -1
#define START_SIZE_OF_TABLE 1000
#define ADD_CONST 1


struct Hash_table
{
    char Key[100]; // todo подумать как сделать массив динамическим
    int Num; // количество строк в файле
};
typedef struct Hash_table H_table;

int ReadWord(FILE *f, char *Str)
{
    int byte = 0, n = 0;

    do // читаем файл до появления первого символа строки
    {
        byte = fgetc(f);
    }
    while((SIMBOL((char)byte)) && (byte != EOF));

    while(byte != EOF)
    {
        if(SIMBOL((char)byte))
        {
            *(Str + n) = '\0';
            return(1);
        }

        else
        {
            *(Str + n) = (char)byte;
            n++;
        }
        byte = fgetc(f);
    }
    *(Str + n) = '\0';
    return(1);
}

int GetHash(char *Str, int SizeOfTab)
{
    int n = 0;
    long long my_hash = 0;
    int Hash;


    while(Str[n] != '\0')
    {
        my_hash += (unsigned)Str[n];

        n++;
        if(n > 100)
            break;
    }
    Hash = my_hash % SizeOfTab;
    return(Hash);
}

void PlotHashTable(H_table *Htable, int SizeOfTable)
{
    for(int i = 0; i < SizeOfTable; i++) //
    {
        printf("Hash = %d \tNum = %d \tKey = %s\n", i, Htable[i].Num, Htable[i].Key);
    }
}

void PlotWords(H_table *Htable, int SizeOfTable)
{
    for(int i = 0; i < SizeOfTable; i++) //
    {
        if(Htable[i].Key[0])
            printf("Word count = %d \t\tKey = %s\n", Htable[i].Num, Htable[i].Key);
    }
}

int SearchFreeCell(int Hash, H_table *Htable, int SizeOfTable) // поиск свободной ячейки
{
    for(int i = 0; i < SizeOfTable; i++) // проходим всю таблицу
    {
        Hash++;                             // инкрементируя хеш
        if(Hash == SizeOfTable)             // если дошли до края таблицы,
            Hash = 0;                          // перемещаемся в ее начало
        if(Htable[Hash].Key[0] == 0)        // если нашли свободную ячейку
            return(Hash);                       // выходим со значением найденного хеша
    }
    return(NOT);                          // если прошли всю таблицу и не нашли св ячейки, выходим с ошибкой
}

H_table *TableExtansion(H_table *Htab, int SizeOfTab)
{
    int NewHash;
    H_table *Buffer = (H_table*)calloc(SizeOfTab - ADD_CONST, sizeof(H_table)); // временное хранилище для переопределения памяти
    for(int j = 0; j < SizeOfTab - ADD_CONST; j++)                     // копируем таблицу в буффер
    {
        strcpy(Buffer[j].Key, Htab[j].Key);
        Buffer[j].Num = Htab[j].Num;
    }
    free(Htab);                                            // освобождаем память старой таблицы
    H_table *NewHtab = (H_table*)calloc(SizeOfTab, sizeof(H_table)); // переопределяем память под новую таблицу

    for(int j = 0; j < SizeOfTab - ADD_CONST; j++)          // проходим по буфферу,
    {
        NewHash = GetHash(Buffer[j].Key, SizeOfTab);                // вычисляем новый хеш
        //if(NewHtab[NewHash].Key[0])                                 // если элемент таблицы занят
        if(NewHtab[NewHash].Key[0] && strcmp(NewHtab[NewHash].Key, Buffer[j].Key)) // если элемент таблицы занят, и в нем не наша строка
        {
            NewHash = SearchFreeCell(NewHash, NewHtab, SizeOfTab);       // ищем хеш по новой таблице
            if(NewHash == NOT)
                printf("ERROR NewHash == NOT");
            strcpy(NewHtab[NewHash].Key, Buffer[j].Key);                   // и как только нашли, записываем в нее информацию
            NewHtab[NewHash].Num = Buffer[j].Num;
        }
        else // иначе, если элемент таблицы свободен, заполняем его
        {
            strcpy(NewHtab[NewHash].Key, Buffer[j].Key);
            NewHtab[NewHash].Num = Buffer[j].Num;
        }
    }
    free(Buffer);
    return(NewHtab);
}


/*****************************************************************************************************************************/
/*****************************************************************************************************************************/

int main(int argc, char *argv[])
{
    int LenStr = 0;
    int Hash;
    char str[50] = {0}; // считаем, что число символов в строке не больше 50
    int SizeOfTable = START_SIZE_OF_TABLE; // начальный размер таблицы
    H_table *Htable = (H_table*)calloc(SizeOfTable, sizeof(H_table)); // выделяем память

    if(argc != 2)
    {
    	printf("Please enter command line arguments in the form: Program_name File_name\n");
    	return(0);
    }

    FILE *fin = fopen(argv[1], "r");

    if(fin == NULL)
    {
        printf("ERROR openning file\n");
        return 0;
    }

    while(1)
    //for(int i = 0; i < 59; i++)
    {
        if(feof(fin))
            break;
        LenStr += ReadWord(fin, str); // читаем слово и инкрементируем счетчик для статистики (в алгоритме не участвует)
        Hash = GetHash(str, SizeOfTable);
        if(Htable[Hash].Key[0]) // если элемент таблицы занят, проверяем что в нем
        {
            if(!strcmp(Htable[Hash].Key, str)) // если ключ и считанная строка совпадают
            {
                Htable[Hash].Num++; //увеличиваем значение (количество встреченных строк) по этому ключу
            }
            else  // в противном случае у нас коллизия
            {

                Hash = SearchFreeCell(Hash, Htable, SizeOfTable); // ищем свободную ячейку
                if(Hash == NOT) // в таблице нет свободных ячеек
                {
                    Htable = TableExtansion(Htable, SizeOfTable += ADD_CONST); // расширяем таблицу
                    Hash = GetHash(str, SizeOfTable); // вычисляем новый хеш
                    if(Htable[Hash].Key[0] && strcmp(Htable[Hash].Key, str)) // если элемент таблицы занят, и в нем не наша строка
                        Hash = SearchFreeCell(Hash, Htable, SizeOfTable); // ищем свободную ячейку
                    if(Hash == NOT)                  // тут по идее такого произойти не должно,
                        printf("ERROR Hash == NOT");      // но на всякий случай поставлю маркер
                    strcpy(Htable[Hash].Key, str);       // и как только нашли, записываем в нее информацию
                    Htable[Hash].Num++;
                }
                else
                {
                    strcpy(Htable[Hash].Key, str);       // и как только нашли, записываем в нее информацию
                    Htable[Hash].Num++;
                }
            }
        }
        else // иначе, если свободен - просто записываем
        {
            strcpy(Htable[Hash].Key, str);
            Htable[Hash].Num++;
        }
    }
    fclose(fin);

    PlotWords(Htable, SizeOfTable);

    return 0;
}
