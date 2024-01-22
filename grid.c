#include "grid.h"
#include "data.h"
#include "mpi.h"

Grid* Grid__create(int typeOfAnalysis, char* rule, int nbGenerations, int nbProc)
{
    Grid* grid = NULL;
    grid = (Grid*)malloc(sizeof(Grid));
    grid->NGeneration = NULL;
    grid->NMinusOneGeneration = NULL;
    grid->NMinusTwoGeneration = NULL;
    grid->gridSize = 0;
    grid->typeOfAnalysis = typeOfAnalysis;
    strcpy(grid->rule, rule);
    grid->rule[4] = '\0';
    grid->nbGenerations = nbGenerations;
    grid->nbProc = nbProc;
    grid->receivedData = NULL;
    grid->tasksPerProc = NULL;
    
    return grid;
}

int Grid__getGridSize(Grid* self)
{
    return self->gridSize;
}

int Grid__getNbGeneration(Grid* self)
{
    return self->nbGenerations;
}

void Grid__setGridSize(Grid* self, int gridSize)
{
    self->gridSize = gridSize;
}

void Grid__setTasksPerProc(Grid* self, int* tasksPerProc)
{
    self->tasksPerProc = tasksPerProc;
}

void Grid__fillGrid(Grid* self)
{
    //On récupère la taille du cube
    int gridSize = Grid__getGridSize(self);
    //On alloue un espace mémoire dans le tas pour créer le tableau NGeneration
    self->NGeneration = (char***)malloc((size_t)gridSize*sizeof(char**));
    for(int i = 0; i < gridSize; i++)
    {
        self->NGeneration[i] = (char**)malloc((size_t)gridSize*sizeof(char*));
        for(int j = 0; j < gridSize; j++)
        {
            self->NGeneration[i][j] = (char*)malloc((size_t)(gridSize+1)*sizeof(char));
        }
    }
    //Et on le remplit
    for(int i = 0; i < gridSize; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            for(int k = 0; k < gridSize; k++)
            {
                self->NGeneration[i][j][k] = (char)(rand() % 2 + 48);
            }
            self->NGeneration[i][j][gridSize] = '\0';
        }
    }
}

void Grid__displayGrid(char*** currentGen, int gridSize, int nbPlanes)
{
    printf("Affichage d'une generation\n\n");
    for(int i = 0; i < nbPlanes; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            for(int k = 0; k < gridSize; k++)
            {
                if(currentGen[i][j][k] == '0')
                    printf(".");
                else
                    printf("%c", currentGen[i][j][k]);
            }
            printf("\n");
        }
        printf("\n\n");
    }
}

void Grid__setToDefault(Grid* self)
{
    //On récupère la taille de la grille
    //On libère la mémoire allouée pour la grille NGeneration
    int gridSize = Grid__getGridSize(self);
    for(int i = 0; i < gridSize; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            free(self->NGeneration[i][j]);
        }
        free(self->NGeneration[i]);
    }
    free(self->NGeneration);
    self->NGeneration = NULL;

    //On libère la mémoire allouée pour la grille NMinusOneGeneration
    if(self->NMinusOneGeneration != NULL)
    {
        for(int i = 0; i < gridSize; i++)
        {
            for(int j = 0; j < gridSize; j++)
            {
                free(self->NMinusOneGeneration[i][j]);
            }
            free(self->NMinusOneGeneration[i]);
        }
        free(self->NMinusOneGeneration);
        self->NMinusOneGeneration = NULL;
    }

    //On libère la mémoire allouée pour la grille NMinusTwoGeneration
    if(self->NMinusTwoGeneration != NULL)
    {
        for(int i = 0; i < gridSize; i++)
        {
            for(int j = 0; j < gridSize; j++)
            {
                free(self->NMinusTwoGeneration[i][j]);
            }
            free(self->NMinusTwoGeneration[i]);
        }
        free(self->NMinusTwoGeneration);
        self->NMinusTwoGeneration = NULL;
    }

    //On récupère le nombre de processus utilisés
    int nbProc = self->nbProc;
    for(int i = 0; i < nbProc - 1; i++)
    {
        free(self->receivedData[i]);
    }
    free(self->receivedData);
    self->receivedData = NULL;
}

int Grid__compareGen(Grid* self, char*** newGen)
{
    //On récupère la taille du cube
    int gridSize = Grid__getGridSize(self);
    //Un booléen pour arrêter la recherche
    int isSame = 1;

    int i = 0;
    while(i < gridSize && isSame)
    {
        int j = 0;
        while(j < gridSize && isSame)
        {
            int k = 0;
            while(k < gridSize && isSame)
            {
                if(self->NGeneration[i][j][k] != newGen[i][j][k])
                    isSame = 0;
                else
                    k++;
            }
            j++;
        }
        i++;
    }

    //Si les patterns coïncident ici on retourne 1(configuration stable)
    if(isSame)
        return 1;
    //sinon on regarde si la variable NMinusOneGeneration contient un pattern
    else if(self->NMinusOneGeneration != NULL)
    {
        isSame = 1;
        i = 0;
        while(i < gridSize && isSame)
        {
            int j = 0;
            while(j < gridSize && isSame)
            {
                int k = 0;
                while(k < gridSize && isSame)
                {
                    if(self->NMinusOneGeneration[i][j][k] != newGen[i][j][k])
                        isSame = 0;
                    else
                        k++;
                }
                j++;
            }
            i++;
        }

        //Si les patterns coïncident ici on retourne 2(configuration stable oscillatoire de période 2)
        if(isSame)
            return 2;
        //sinon on regarde si la variable NMinusTwoGeneration contient un pattern
        else if(self->NMinusTwoGeneration != NULL)
        {
            isSame = 1;
            i = 0;
            while(i < gridSize && isSame)
            {
                int j = 0;
                while(j < gridSize && isSame)
                {
                    int k = 0;
                    while(k < gridSize && isSame)
                    {
                        if(self->NMinusTwoGeneration[i][j][k] != newGen[i][j][k])
                            isSame = 0;
                        else
                            k++;
                    }
                    j++;
                }
                i++;
            }
            //Si les patterns coïncident ici on retourne 2(configuration stable oscillatoire de période 3)
            if(isSame)
                return 3;
            else
                return 0;
        }
        else
            return 0;
    }
    else
        return 0;
}

int Grid__isEmpty(char*** newGen, int gridSize)
{
    //On crée un booléen pour savoir si le tableau est vide
    int isEmpty = 1;

    int i = 0;
    while(i < gridSize && isEmpty)
    {
        int j = 0;
        while(j < gridSize && isEmpty)
        {
            int k = 0;
            //Dès que l'on trouve une case contenant un 1, alors la grille de cellules n'est pas vide
            while(k < gridSize && isEmpty)
            {
                if(newGen[i][j][k] == '1')
                    isEmpty = 0;
                else
                    k++;
            }
            j++;
        }
        i++;
    }

    return isEmpty;
}

int Grid__levelIsEmpty(char** level, int gridSize)
{
     //On crée un booléen pour savoir si un feuillet est vide
    int isEmpty = 1;

    int i = 0;
    while(i < gridSize && isEmpty)
    {
        int j = 0;
        //Dès que l'on trouve une case contenant un 1, alors le feuillet n'est pas vide 
        while(j < gridSize && isEmpty)
        {
            if(level[i][j] == '1')
                isEmpty = 0;
            else
                j++;
        }
        i++;
    }

    return isEmpty;
}

void Grid__updatePatterns(Grid* self, char*** newGen)
{
    //On vérifie si la variable NMinusTwoGeneration ne contient pas la valeur NULL (cas de la generation 1 et 2)
    //Cas de la génération 3 ou plus
    if(self->NMinusTwoGeneration != NULL)
    {
        //On récupère la taille du cube
        int gridSize = Grid__getGridSize(self);
        //On libère la mémoire allouée pour ce pattern
        for(int i = 0; i < gridSize; i++)
        {
            for(int j = 0; j < gridSize; j++)
            {
                free(self->NMinusTwoGeneration[i][j]);
            }
            free(self->NMinusTwoGeneration[i]);
        }
        free(self->NMinusTwoGeneration);
        self->NMinusTwoGeneration = NULL;
        self->NMinusTwoGeneration = self->NMinusOneGeneration;
    }
    else
        //Cas de la génération 2
        if(self->NMinusOneGeneration != NULL)
            self->NMinusTwoGeneration = self->NMinusOneGeneration;

    
    //le pattern de la génération n devient le pattern de la génération n - 1
    self->NMinusOneGeneration = self->NGeneration;

    //Le nouveau pattern devient le pattern de la génération n
    self->NGeneration = newGen; 

}

int* Grid__analyzePattern(Grid* self)
{
    //on récupère la taille du cube
    int gridSize = Grid__getGridSize(self);
    //ainsi que le nombre de générations sur lequel rechercher une configuration stable
    int nbGenerations = Grid__getNbGeneration(self);
    //On crée un compteur sur le nombre de générations
    int genCount = 2;
    //une variable qui contient le résultat retourné par la fonction
    int result = 0;
    //et un booléen indiquant si oui ou non on a trouvé une configuarion stable
    int findConfig = 0;
    MPI_Status stat;
    while(genCount < nbGenerations && !findConfig)
    {
        char** data = Grid__createDataPerProc(self);
        for(int k = 0; k < self->nbProc - 1; k++)
        {
            //On récupère la taille des données à envoyer
            int bufferSize = (int)strlen(data[k]) + 1;
            char buffer[bufferSize];
            //Et on copie les données à envoyer dans ce buffer
            strcpy(buffer, data[k]);
            //buffer -> données à envoyer
            //count -> nombre d'éléments à envoyer
            //datatype -> type de données à envoyer
            //dest -> rang du processus auquel envoyer les données
            //tag -> tag du message (entier)
            //comm -> communicateur utilisé pour l'envoi des données
            MPI_Send(buffer, bufferSize, MPI_BYTE, k + 1, 0, MPI_COMM_WORLD);
        }
        //On libère la mémoire allouée pour l'envoi du données
        for(int k = 0; k < self->nbProc - 1; k++)
        {
            free(data[k]);
        }
        free(data);
        data = NULL;

        //On initialise le tableau des données envoyées par les différents processus
        Grid__initializeReceivedDataTab(self);
        for(int k = 0; k < self->nbProc - 1; k++)
        {
            //buffer -> buffer pour stocker les données reçues
            //count -> nombre d'éléments à recevoir
            //datatype -> type de données à recevoir
            //source -> rang du processus qui a envoyé les données
            //tag -> tag du message (entier)
            //comm -> communicateur utilisé pour la reception des données
            //status -> le status de l'objet
            MPI_Recv(self->receivedData[k], 3375001, MPI_BYTE, k+1, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
        }
        //On retransforme les données reçues en grille 3d
        char*** tabTemp = Grid__dataToGrid(self);
        //Si le pattern tabTemp est vide, alors on libère la mémoire allouée à ce tableau et on sort prématurément de la boucle while
        if(Grid__isEmpty(tabTemp, self->gridSize))
        {
            for(int i = 0; i < gridSize; i++)
            {
                for(int j = 0; j < gridSize; j++)
                {
                    free(tabTemp[i][j]);
                }
                free(tabTemp[i]);
            }
            free(tabTemp);
            tabTemp = NULL;
            break;
        }
        //Et on le compare avec les patterns des générations précédentes en fonction du nombre de générations passées
        result = Grid__compareGen(self, tabTemp);
        //Si on a trouvé un resultat probant
        if(result)
        {
            //On change la valeur du booléen afin de sortir de la boucle
            findConfig = 1;
            printf("#### Une correspondance a ete trouvee entre la grille de cellules de la generation %d et une generation de cellules anterieure. ####\n", genCount);

            //et on libère la mémoire allouée pour le tableau temporaire
            for(int i = 0; i < gridSize; i++)
            {
                for(int j = 0; j < gridSize; j++)
                {
                    free(tabTemp[i][j]);
                }
                free(tabTemp[i]);
            }
            free(tabTemp);
            tabTemp = NULL;
        }
        //Sinon on incrémente le compteur et on met à jour les patterns des différentes générations
        else
        {
            Grid__updatePatterns(self, tabTemp);
            if(self->typeOfAnalysis == 1)
                printf("Pas de resultat trouve a la generation %d.\n", genCount);
            genCount++;
        }
    }

    //On crée et on retourne notre tableau de retour avec le résultat et la génération
    int* tabResults = (int*)malloc(2*sizeof(int));

    //Cas particulier où il y a encore des cellules vivantes mais aucune configuration stable n'a été trouvée dans le nombre de générations imparti
    if(result == 0 && genCount == nbGenerations)
        tabResults[0] = 4;
    else
        tabResults[0] = result;
    //Si on a trouvé une configuration oscillatoire de période 3   
    if(result == 3)
        genCount = genCount - 3;
    //Sinon si on a trouvé une configuration oscillatoire de période 2  
    else if(result == 2)
        genCount = genCount - 2;
    //Sinon si on a trouvé une configuration stable ou bienpas de configuration pour le nombre de générations voulu
    else if(result == 1 || result == 4)
        genCount--;
    
    tabResults[1] = genCount;
    
    return tabResults;
}

char** Grid__createDataPerProc(Grid* self)
{
    //On récupère la taille de la grille au format numérique
    int gridSize = Grid__getGridSize(self);
    //ainsi que le nombre de processus utilisés
    int nbProc = self->nbProc;
    //ainsi que sous la forme d'une chaine de caractères combien de caractères seront réservés pour la taille de la grille
    char buffer[10];
    sprintf(buffer, "_%d", gridSize);
    //On crée le tableau qui contiendra les portions de la grille destinées aux différents processus
    char** data = NULL;
    data = (char**)malloc((size_t)(nbProc-1)*sizeof(char*));
    //On crée un compteur sur le nombre de feuillets stockés
    int count = 0;
    for(int i = 0; i < nbProc - 1; i++)
    {
        size_t bufferSize = (size_t)(self->tasksPerProc[i]*gridSize*gridSize + 7) + strlen(buffer);
        //On récupère la taille des données sous la forme d'une chaine de caractères
        char dataSize[10];
        sprintf(dataSize, "_%d_", self->tasksPerProc[i]*gridSize*gridSize);
        //Pour la composition du "datagramme" qui sera envoyé à chaque processus, le format sera le suivant
        //modèle (4 char) + underscore (1 char) + taille de la grille (au moins 1 char) + underscore (1 char) + taille des données stockées (au moins 1 char) + underscore (1 char) + données
        //Exemple de datagramme: 5766_3_18_011000100110011101
        data[i] = (char*)malloc((bufferSize+strlen(dataSize))*sizeof(char));
        //On stocke les informations du modèle, de la taille de la grille, et de la taille des données
        strcpy(data[i], self->rule);
        strcat(data[i], buffer);
        strcat(data[i], dataSize);

        //Et on stocke les données
        for(int j = 0; j < self->tasksPerProc[i]; j++)
        {
            for(int k = 0; k < gridSize; k++)
            {
                strcat(data[i], self->NGeneration[j+count][k]);
            }
        }
        //On actualise la valeur de count du nombre de feuillet pour ce processus - 1 afin d'avoir les voisins des prochains feuillets stockés
        count = count + self->tasksPerProc[i] - 2;       
    }

    return data;
}

void Grid__initializeReceivedDataTab(Grid* self)
{
    //On récupère la taille de la grille au format numérique
    int gridSize = Grid__getGridSize(self);
    //ainsi que le nombre de processus utilisés
    int nbProc = self->nbProc;
    self->receivedData = (char**)malloc((size_t)(nbProc-1)*sizeof(char*));
    if((nbProc - 1 > 1))
    {
        for(int i = 0; i < nbProc - 1; i++)
        {
            //S'il s'agit du premier ou du dernier processus
            if(i == 0 || i == (nbProc - 2))
                self->receivedData[i] = (char*)malloc((size_t)((self->tasksPerProc[i]-1)*gridSize*gridSize + 7)*sizeof(char));
            else
                self->receivedData[i] = (char*)malloc((size_t)((self->tasksPerProc[i]-2)*gridSize*gridSize + 7)*sizeof(char));
        }
    }
    else
        self->receivedData[0] = (char*)malloc((size_t)(self->tasksPerProc[0]*gridSize*gridSize + 7)*sizeof(char));
}

char*** Grid__dataToGrid(Grid* self)
{
    //On récupère la taille de la grille
    int gridSize = Grid__getGridSize(self);
    //Afin de faciliter le passage des données d'un char** vers un char***, on va tout d'abord passer
    //les données d'un char** vers un char* et ensuite transformer ces données dans le char*
    //vers un char***
    //On commence par créer le tableau de données
    char* data = NULL;
    data = (char*)malloc((size_t)(gridSize*gridSize*gridSize + 1)*sizeof(char));
    strcpy(data, "");
    //Et on le remplit avec les données reçues par le processus 0
    for(int i = 0; i < self->nbProc - 1; i++)
    {
        strcat(data, self->receivedData[i]);
    }
    data[gridSize*gridSize*gridSize] = '\0';

    //Puis on crée la grille qui contiendra les données reçues
    char*** newGen = NULL;
    newGen = (char***)malloc((size_t)gridSize*sizeof(char**));
    for(int i = 0; i < gridSize; i++)
    {
        newGen[i] = (char**)malloc((size_t)gridSize*sizeof(char*));
        for(int j = 0; j < gridSize; j++)
        {
            newGen[i][j] = (char*)malloc((size_t)(gridSize + 1)*sizeof(char));
        }
    }
    //Et on remplit la grille avec les données contenues dans data
    //On crée un compteur sur les données de data
    int count = 0;
    //Et on remplit la grille
    for(int i = 0; i < gridSize; i++)
    {
        for(int j = 0; j < gridSize; j++)
        {
            for(int k = 0; k < gridSize; k++)
            {
                newGen[i][j][k] = data[count];
                count++;
            }
            newGen[i][j][gridSize] = '\0';
        }
    }
    //Enfin, on libère la mémoire allouée par data
    free(data);
    data = NULL;

    return newGen;
}

void Grid__destroy(Grid** self)
{
    //On récupère la taille de la grille
    //On libère la mémoire allouée pour la grille NGeneration
    int gridSize = Grid__getGridSize(*self);
    if((*self)->NGeneration != NULL)
    {
        for(int i = 0; i < gridSize; i++)
        {
            for(int j = 0; j < gridSize; j++)
            {
                free((*self)->NGeneration[i][j]);
            }
            free((*self)->NGeneration[i]);
        }
        free((*self)->NGeneration);
        (*self)->NGeneration = NULL;
    }

    //On libère la mémoire allouée pour la grille NMinusOneGeneration
    if((*self)->NMinusOneGeneration != NULL)
    {
        for(int i = 0; i < gridSize; i++)
        {
            for(int j = 0; j < gridSize; j++)
            {
                free((*self)->NMinusOneGeneration[i][j]);
            }
            free((*self)->NMinusOneGeneration[i]);
        }
        free((*self)->NMinusOneGeneration);
        (*self)->NMinusOneGeneration = NULL;
    }

    //On libère la mémoire allouée pour la grille NMinusTwoGeneration
    if((*self)->NMinusTwoGeneration != NULL)
    {
        for(int i = 0; i < gridSize; i++)
        {
            for(int j = 0; j < gridSize; j++)
            {
                free((*self)->NMinusTwoGeneration[i][j]);
            }
            free((*self)->NMinusTwoGeneration[i]);
        }
        free((*self)->NMinusTwoGeneration);
        (*self)->NMinusTwoGeneration = NULL;
    }

    //On libère également la mémoire allouée pour le tableau des données reçues
    if((*self)->receivedData != NULL)
    {
        for(int i = 0; i < (*self)->nbProc - 1; i++)
        {
            free((*self)->receivedData[i]);
        }
        free((*self)->receivedData);
        (*self)->receivedData = NULL;
    }

    //On met le contenu de l'attribut tasksPerProc à NULL
    (*self)->tasksPerProc = NULL;

    free(*self);
    *self = NULL;
}