#include "data.h"
#include "grid.h"

Data* Data__create(char* data)
{
    Data* d = (Data*)malloc(sizeof(Data));
    d->grid = NULL;
    Data__storeData(d, data);

    return d;
}

int Data__getGridSize(Data* self)
{
    return self->gridSize;
}

void Data__storeData(Data* self, char* data)
{
    //On commence par récupérer le modèle utilisé (les 4 premiers char du message)
    memcpy(self->rule, data, 4);
    self->rule[4] = '\0';
    //On récupère la position de tous les caractères spéciaux dans le message
    char* underscore_pos1 = strchr(data, '_');
    char* underscore_pos2 = strchr(data + (int)(underscore_pos1 + 1 - data), '_');
    char* underscore_pos3 = strchr(data + (int)(underscore_pos2 + 1 - data), '_');

    //puis, on récupère la taille de la grille qui commence après le premier caractère '_' et se termine avant le second '_'   
    //que l'on stocke sous la forme d'une chaine de caractère dans la buffer
    char bufferSize[10];
    strncpy(bufferSize, underscore_pos1 + 1, (size_t)(underscore_pos2 - underscore_pos1));
    //et sous forme numérique dans l'attribut gridSize
    self->gridSize = atoi(bufferSize);
    
    //On fait de même pour la longueur des données contenues dans la chaine de caractères data
    char bufferLength[10];
    strncpy(bufferLength, underscore_pos2 + 1, (size_t)(underscore_pos3 - underscore_pos2));
    int dataLength = atoi(bufferLength);

    //On calcule le nombre de feuillet qu'à reçu le processus
    self->nbPlanes = dataLength / (self->gridSize*self->gridSize);
    //On stocke la partie du contenu de data réservée aux données dans un buffer
    char bufferData[dataLength + 1];
    strncpy(bufferData, underscore_pos3 + 1, (size_t)dataLength);
    bufferData[dataLength] = '\0';
    //Et on recrée la grille en fonction de ces données
    Data__dataToGrid(self, bufferData);
}

void Data__dataToGrid(Data* self, char* data)
{
    //On récupère la taille de la grille
    int gridSize = Data__getGridSize(self);
    //On alloue dans la tas la grille de ce processeur en fonction de la taille de la grille
    self->grid = (char***)malloc((size_t)self->nbPlanes*sizeof(char**));
    for(int i = 0; i < self->nbPlanes; i++)
    {
        self->grid[i] = (char**)malloc((size_t)gridSize*sizeof(char*));
        for(int j = 0; j < gridSize; j++)
        {
            self->grid[i][j] = (char*)malloc((size_t)(gridSize + 1)*sizeof(char));
        }
    }

    //On crée un itérateur sur les données à stocker
    int count = 0;
    //Et on remplit la grille
    for(int i = 0; i < self->nbPlanes; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            for(int k = 0; k < gridSize; k++)
            {
                self->grid[i][j][k] = data[count];
                count++;
            }
            self->grid[i][j][gridSize] = '\0';
        }
    }
}

char* Data__setNewGen(Data* self, int rank, int nbProc)
{
    //On détermine la nouvelle génération de cellules
    char*** newGen = NULL;
    //S'il n'y a seulement qu'un processus pour les tâches parallélisables
    if(nbProc - 1 == 1)
        newGen = Data__cellNextGen(self, 0, self->nbPlanes);
    else
    {
        if(rank == 1)
            newGen = Data__cellNextGen(self, 0, self->nbPlanes - 1);
        else if(rank == nbProc - 1)
            newGen = Data__cellNextGen(self, 1, self->nbPlanes);
        else
            newGen = Data__cellNextGen(self, 1, self->nbPlanes - 1);
    }

    //Puis on transforme la nouvelle génération d'une forme matricielle à 3 dimensions en données prêtes à être envoyées au processeur 0 (forme séquentielle)
    char* data = NULL;
    //On vérifie le nombre de feuillet à envoyer en fonction du numéro du processeur
    //On récupère le nombre de feuillets à expédier au processeur 0
    //Si on utilise qu'un seul processeur pour l'exécution de cette tâche (c'est-à-dire le nombre de processeur total - le processeur 0)
    //On stocke l'intégralité des feuillets de la nouvelle génération
    if((nbProc - 1) == 1)
        data = Data__gridToData(newGen, self->gridSize, self->nbPlanes, 0, self->nbPlanes);
    //Sinon, il y a plus d'un processeur et on stocke un certain nombre de feuillets en fonction du rang du processeur
    else
    {
        int planesToSend = 0;
        //S'il s'agit du premier ou du dernier processeur à réaliser les tâches parallélisables
        if(rank == 1 || rank == nbProc - 1)
            planesToSend = self->nbPlanes - 1;
        else
            planesToSend = self->nbPlanes - 2;

        //Puis, on transforme la grille en données sous la forme d'une chaine de caractères en fonction du rang du processeur
        if(rank == 1)
            data = Data__gridToData(newGen, self->gridSize, planesToSend, 0, planesToSend);
        else
            data = Data__gridToData(newGen, self->gridSize, planesToSend, 1, planesToSend + 1);
    }
    //On libère l'espace mémoire alloué pour la grille de la nouvelle génération
    for(int i = 0; i < self->nbPlanes; i++)
    {
        for(int j = 0; j < self->gridSize; j++)
        {
            free(newGen[i][j]);
        }
        free(newGen[i]);
    }
    free(newGen);
    newGen = NULL;

    return data;
}

char* Data__gridToData(char*** grid, int gridSize, int nbPlanes, int begin, int end)
{
    //On calcule la taille des données à envoyer au processeur 0
    int dataSize = nbPlanes*gridSize*gridSize + 1;
    //On crée la chaine de caractères qui contiendra les données
    char* data = (char*)malloc((size_t)(dataSize)*sizeof(char));
    //Puis, on stocke le contenu de la grille que l'on souhaite retourner dans cette chaine de caractères
    strcpy(data, "");
    for(int i = begin; i < end; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            strcat(data, grid[i][j]);
        }
    }
    data[dataSize - 1] = '\0';
    return data;
}

void Data__destroy(Data** self)
{
    if((*self)->grid != NULL)
    {
        for(int i = 0; i < (*self)->nbPlanes; i++)
        {
            for(int j = 0; j < (*self)->gridSize; j++)
            {
                free((*self)->grid[i][j]);
            }
            free((*self)->grid[i]);
        }
        free((*self)->grid);
        (*self)->grid = NULL;
    }
    free(*self);
    *self = NULL;
}

//compte le nombre de cellules vivantes adjacentes à celle de coordonnées (x, y, z)
//et retourne 0 ou 1 selon que la cellule sera morte ou vivante à la génération suivante
char*** Data__cellNextGen(Data* self, int begin, int end)
{
    //On récupère la taille de la grille
    int gridSize = self->gridSize;
    //ainsi que le nombre de feuillet
    int nbPlanes = self->nbPlanes;
    //et la grille de la génération actuelle
    char*** currentGen = self->grid;
    //On instancie un tableau temporaire qui contient les cellules de la génération suivante 
    char*** tabTemp = (char***)malloc((size_t)nbPlanes * sizeof(char**));
    for(int i = 0; i < nbPlanes; i++)
    {
        tabTemp[i] = (char**)malloc((size_t)gridSize * sizeof(char*));
        for(int j = 0; j < gridSize; j++)
        {
            tabTemp[i][j] = (char*)malloc((size_t)(gridSize + 1) * sizeof(char));
        }
    }

    //Puis on va directement recopier dans ce tableau les feuillets voisins qui ne seront pas pris en compte dans la nouvelle génération de cellules
    if(begin != 0)
    {
        for(int j = 0; j < gridSize; j++)
        {
            strcpy(tabTemp[0][j], "");
            strcat(tabTemp[0][j], currentGen[0][j]);
        }
    }

    if(end != nbPlanes)
    {
        for(int j = 0; j < gridSize; j++)
        {
            strcpy(tabTemp[nbPlanes - 1][j], "");
            strcat(tabTemp[nbPlanes - 1][j], currentGen[nbPlanes - 1][j]);
        }
    }
    //printf("Hello2\n");
    //On récupère également les bornes du modèle choisi par l'utilisateur
    int borne1Inf = self->rule[0] - 48;
    int borne1Sup = self->rule[1] - 48;
    int borne2Inf = self->rule[2] - 48;
    int borne2Sup = self->rule[3] - 48;

    //Puis on parcours l'ensemble des cellules de la grille en comptant le nombre de cellules vivantes adjacentes à chacune d'entre elles 
    for(int i = begin; i < end; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            for(int k = 0; k < gridSize; k++)
            {
                int count = 0;
                //en haut
                if(i == 0)
                {
                    //à gauche
                    if(j == 0)
                    {
                        //dans le coin supérieur gauche avant
                        /*     +--------+
                              /        /|
                             /        / |
                            @--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        if(k == 0)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //4 cases au niveau inférieur i + 1
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;                           
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            
                        }
                        //dans le coin supérieur gauche arrière
                        /*     @--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //3 cases au même niveau que la case en question
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //4 cases sur le niveau inférieur i + 1
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;                            
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            
                        }
                        //sinon sur l'arête supérieure gauche
                        /*     +--------+
                              @        /|
                             @        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else
                        {
                            //5 cases au niveau i
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //6 cases au niveau i + 1
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                        }
                    }
                    //à droite
                    else if(j == gridSize - 1)
                    {
                        //dans le coin supérieur droit avant
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------@  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        if(k == 0)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //4 cases sur le niveau inférieur i + 1                          
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;                           
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;                           
                        }
                        //dans le coin supérieur droit arrière
                        /*     +--------@
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //4 cases sur le niveau inférieur i + 1                           
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;                           
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;                           
                        }
                        //sinon sur l'arête supérieure droite
                        /*     +--------+
                              /        @|
                             /        @ |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else
                        {
                            //5 cases au niveau i
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //6 cases au niveau i + 1
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;

                        }
                    }
                    //sinon dans la face supérieure
                    else
                    {
                        //sur l'arête avant
                        /*     +--------+
                              /        /|
                             /        / |
                            +@@@@@@@@+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        if(k == 0)
                        {
                            //5 cases sur le même niveau (i) que la case en question
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            //6 cases sur le niveau inférieur (i + 1)
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                        }
                        //sur l'arête arrière
                        /*     +@@@@@@@@+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //5 cases sur le même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            //6 cases sur le niveau inférieur (i + 1)
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                        }
                        //sinon dans la face supérieure
                        /*     +--------+
                              /@@@@@@@@/|
                             /@@@@@@@@/ |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else
                        {
                            //8 cases sur le même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            //9 cases sur le niveau inférieur (i + 1)
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                        }
                    }
                }
                //en bas
                else if(i == nbPlanes - 1)
                {
                    //à gauche
                    if(j == 0)
                    {
                        //dans le coin inférieur gauche avant
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            @--------+ */
                        if(k == 0)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //4 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;                          
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;                           
                        }
                        //dans le coin inférieur gauche arrière
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |  @     |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //4 cases au niveau supérieur i - 1
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;                          
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;                           
                        }
                        //sinon sur l'arête inférieure gauche
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            | @      | /
                            |@       |/
                            +--------+ */
                        else
                        {
                            //5 cases sur le même niveau que la case en question
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //6 cases sur le niveau supérieur (i - 1)
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                        }
                    }
                    //à droite
                    else if(j == gridSize - 1)
                    {
                        //dans le coin inférieur droit avant
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------@ */
                        if(k == 0)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //4 cases au niveau supérieur i - 1                           
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;                           
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;                           
                        }
                        //dans le coin inférieur droit arrière
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  @
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //3 cases au même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //4 cases au niveau supérieur i - 1
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;                           
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;                           
                        }
                        //sinon sur l'arête inférieure droite
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | @
                            |        |@
                            +--------+ */
                        else
                        {
                            //5 cases sur le même niveau que la case en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //6 cases sur le niveau supérieur (i - 1)                           
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;                           
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;                            
                        }
                    }
                    //sinon dans la face inférieure
                    else
                    {
                        //sur l'arête avant
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |        | /
                            |        |/
                            +@@@@@@@@+ */
                        if(k == 0)
                        {
                            //5 cases sur le même niveau que la case en question
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            //6 cases sur le niveau supérieur
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                        }
                        //sur l'arête arrière
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |   @@@@@|@@+
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //5 cases sur le même niveau que la case en question
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            //6 cases sur le niveau supérieur
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                        }
                        //sinon dans la face inférieure
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        |  |
                            |        |  +
                            |  @@@@@@|@/
                            | @@@@@@@|/
                            +--------+ */
                        else
                        {
                            //8 cases sur le même niveau que la case en question
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            //9 cases sur le niveau supérieur
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                        }
                    }
                }
                //ni tout en haut, ni tout en bas
                else
                    //à gauche
                    if(j == 0)
                    {
                        //sur l'arête avant gauche
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            @        |  |
                            @        |  +
                            @        | /
                            @        |/
                            +--------+ */
                        if(k == 0)
                        {
                            //3 cases au même niveau que la case en question                          
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;                            
                            //4 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                            //4 cases sur le niveau inférieur i + 1
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            
                        }
                        //sur l'arête arrière gauche
                        /*     +--------+
                              /@       /|
                             / @      / |
                            +--------+  |
                            |  @     |  |
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //3 cases au même niveau que la case en question
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //4 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                            //4 cases sur le niveau inférieur i + 1
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;                            
                        }
                        //sinon on est dans la face de gauche
                        /*     +--------+
                              /@       /|
                             /@@      / |
                            +--------+  |
                            |@@@     |  |
                            |@@@     |  +
                            |@@      | /
                            |@       |/
                            +--------+ */
                        else
                        {
                            //5 cases au même niveau que la case en question (i)
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            //6 cases au niveau supérieur i - 1
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                            //6 cases au niveau inférieur i + 1                          
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;                            
                        }
                    }
                    //à droite
                    else if(j == gridSize - 1)
                    {
                        //sur l'arête avant droite
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |        @  |
                            |        @  +
                            |        @ /
                            |        @/
                            +--------+ */
                        if(k == 0)
                        {
                            //3 cases au même niveau que celle en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //4 cases au niveau supérieur i - 1
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            //4 cases au niveau inférieur i + 1
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++; 
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;                           
                        }
                        //sur l'arête arrière droite
                        /*     +--------+
                              /        /@
                             /        / @
                            +--------+  @
                            |        |  @
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //3 cases au même niveau que celle en question (i)
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //4 cases au niveau supérieur i - 1
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            //4 cases au niveau inférieur i + 1
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;                           
                        }
                        //sinon on est dans la face de droite
                        /*     +--------+
                              /        /|
                             /        /@|
                            +--------+@@|
                            |        |@@|
                            |        |@@+
                            |        |@/
                            |        |/
                            +--------+ */
                        else
                        {
                            //5 cases au même niveau que la case en question (i)
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            //6 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            //6 cases sur le niveau inférieur i + 1
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;    
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1 ][j - 1][k - 1] == '1')
                                count++;                           
                        }
                    }
                    //sinon on se situe dans la partie centrale du cube
                    else
                    {
                        //sur la face avant
                        /*     +--------+
                              /        /|
                             /        / |
                            +--------+  |
                            |@@@@@@@@|  |
                            |@@@@@@@@|  +
                            |@@@@@@@@| /
                            |@@@@@@@@|/
                            +--------+ */
                        if(k == 0)
                        {
                            //5 cases au même niveau que la case en question (i)
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            //6 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                            //6 cases sur le niveau inférieur i + 1
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;                            
                        }
                        //sur la face arrière
                        /*     +--------+
                              /@@@@@@@@/|
                             / @@@@@@@/@|
                            +--------+@@|
                            |  @@@@@@|@@|
                            |        |  +
                            |        | /
                            |        |/
                            +--------+ */
                        else if(k == gridSize - 1)
                        {
                            //5 cases au même niveau que la case en question (i)
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //6 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                            //6 cases sur le niveau inférieur i + 1  
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++; 
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;                           
                        }
                        //sinon il s'agit de tout point qui n'est pas présent sur une arête ou une face
                        /*     +--------+
                              /@@@@@@@@/|
                             /@@@@@@@@/@|
                            +--------+@@|
                            |@@@@@@@@|@@|
                            |@@@@@@@@|@@+
                            |@@@@@@@@|@/
                            |@@@@@@@@|/
                            +--------+ */
                        else
                        {
                            //8 cases au même niveau que la case en question (i)
                            if(currentGen[i][j + 1][k] == '1')
                                count++;
                            if(currentGen[i][j - 1][k] == '1')
                                count++;
                            if(currentGen[i][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i][j][k + 1] == '1')
                                count++;
                            if(currentGen[i][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i][j][k - 1] == '1')
                                count++;
                            //9 cases sur le niveau supérieur i - 1
                            if(currentGen[i - 1][j - 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j][k] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i - 1][j - 1][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i - 1][j + 1][k - 1] == '1')
                                count++;
                            //9 cases sur le niveau inférieur i + 1                        
                            if(currentGen[i + 1][j + 1][k] == '1')
                                count++;
                            if(currentGen[i + 1][j][k] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k] == '1')
                                count++;     
                            if(currentGen[i + 1][j + 1][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k + 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k + 1] == '1')
                                count++;                         
                            if(currentGen[i + 1][j + 1][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j][k - 1] == '1')
                                count++;
                            if(currentGen[i + 1][j - 1][k - 1] == '1')
                                count++;                           
                        }
                    }
                //on teste la pérénité des cellules vivantes et/ou la naissance de nouvelles cellules avec le modèle choisi par l'utilisateur
                //si une cellule vivante (= 1), possède moins que la borne1Inf ou plus que la borne1Sup cellules vivantes adjacentes, alors elle meurt
                if((count < borne1Inf || count > borne1Sup) && currentGen[i][j][k] == '1')
                    tabTemp[i][j][k] = '0';
                //si une cellule morte (= 0), possède une valeur supérieure ou égale à borne2Inf ou inférieure ou égale à borne2Sup de cellules vivantes adjacentes, alors elle revient à la vie
                else if((count >= borne2Inf && count <= borne2Sup) && currentGen[i][j][k] == '0')
                    tabTemp[i][j][k] = '1';
                //sinon, dans tous les autres cas, elle garde son état de la génération précédente
                else
                    tabTemp[i][j][k] = currentGen[i][j][k];
                
            }
            tabTemp[i][j][gridSize] = '\0';
        }
    }

    return tabTemp;
}