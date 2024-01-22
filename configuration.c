#include "configuration.h"

Configuration* Configuration__create(int nbProc)
{
    Configuration* config = NULL;
    config = (Configuration*)malloc(sizeof(Configuration));
    config->tasksPerProc = NULL;

    Configuration__setParameters(config, nbProc);


    return config;
}

Configuration* Configuration__createSingleAnalysis(int nbProc)
{
    //Comme le test sur une simple seed se fait sur une taille de grille de 50
    //On vérifie que le nombre de processus choisi par l'utilisateur n'excède pas 51 (50 pour la partie du programme qui est parallélisable + le processus 0)
    if(nbProc > 51)
    {
        printf("Le test sur une grille simple se fait sur une grille de taille 50x50x50.\nPar consequent le nombre de processus utilises ne peut pas exceder 51.\n");
        exit(1);
    }
    
    //On crée une configuration fixe d'une taille de grille 50x50x50,de seed 25, de modèle 5766 et de nombre de générations 200
    Configuration* config = NULL;
    config = (Configuration*)malloc(sizeof(Configuration));
    config->tasksPerProc = NULL;
    config->typeOfAnalysis = 1;
    strcpy(config->rule, "5766");
    config->rule[4] = '\0';
    config->seedInfRange = 25;
    config->seedSupRange = 25;
    config->gridSizeInfRange = 50;
    config->gridSizeSupRange = 50;
    config->nbGenerations = 200;

    return config;
}

Configuration* Configuration__createDataRangeAnalysis(int nbProc)
{
    //Comme le test sur une plage de taille de grille allant de 30 à 40
    //On vérifie que le nombre de processus choisi par l'utilisateur n'excède pas 31 (30 pour la partie du programme qui est parallélisable + le processus 0)
    if(nbProc > 31)
    {
        printf("Le test sur une plage de donnees se fait sur une grille de taille minimale 30x30x30.\nPar consequent le nombre de processus utilises ne peut pas exceder 31.\n");
        exit(1);
    }
    
    //On crée une configuration fixe d'une plage de taille de grille allant de 30x30x30 à 40x40x40,
    //d'une plage de seed allant de 0 à 100, de modèle 5766 et de nombre de générations 200
    Configuration* config = NULL;
    config = (Configuration*)malloc(sizeof(Configuration));
    config->tasksPerProc = NULL;
    config->typeOfAnalysis = 2;
    strcpy(config->rule, "5766");
    config->rule[4] = '\0';
    config->seedInfRange = 0;
    config->seedSupRange = 100;
    config->gridSizeInfRange = 30;
    config->gridSizeSupRange = 40;
    config->nbGenerations = 200;

    return config;
}

void Configuration__setParameters(Configuration* self, int nbProc)
{
    //On demande à l'utilisateur le type d'analyse à effectuer
    //1 -> analyse sur une seed et sur une taille de cube précise
    //2 -> analyse sur une plage de seed et une plage de taille de cube précise
    char analysisChoice[20];
    int validChoice;
    do{
        strcpy(analysisChoice, "");
        printf("************************************\n");
        printf("#### Quel type d'analyse souhaitez-vous faire? ####\n(1) Analyse sur une seed et sur une taille de grille precise...\n(2) Analyse sur une plage de seed et une plage de taille de grille...\n");
        scanf("%s", analysisChoice);
        //On vérifie si le choix de l'utilisateur est un nombre
        int isNumber = Configuration__isANumber(analysisChoice);

        if(!isNumber)
        {
            printf("---- La valeur que vous avez entre pour le type d'analyse n'est pas un nombre. ----\n");
            validChoice = 0;
        }
        else
        {
            int choice = atoi(analysisChoice);
            if(choice != 1 && choice != 2)
            {
                printf("---- Le nombre entre pour le choix d'analyse est incorrect. ----\n");
                validChoice = 0;
            }
            else
                validChoice = 1;
        }
    }while(!validChoice);
    self->typeOfAnalysis = atoi(analysisChoice);

    //choix de la règle pour le jeu de la vie
    char ruleChoice[20];
    do{
        strcpy(ruleChoice, "");
        printf("************************************\n");
        printf("#### Quelle regle voulez-vous utiliser pour l'analyse des cellules? ####\n");
        printf("#### Le format de la regle est compose de 4 chiffres: les deux premiers (resp. les deux derniers) correspondent aux bornes inferieure et superieure des nombres de cellules vivantes adjacentes pour permettre a une cellule vivante de rester en vie (resp. pour permettre a une cellule morte de revenir a la vie) ####\n");
        printf("Note1: le second et le quatrieme chiffre doivent etre respectivement superieur ou egal au premier et au troisieme chiffre.\nNote2: la difference entre le second et le premier chiffre (respectivement le quatrieme et le troisieme) ne doit pas exceder 2.\n");
        scanf("%s", ruleChoice);
        int isNumber = Configuration__isANumber(ruleChoice);
        if(!isNumber)
        {
            printf("---- La valeur que vous avez entre pour le choix du modele n'est pas un nombre. ----\n");
            validChoice = 0;
        }
        else
            if(strlen(ruleChoice) != 4 || ruleChoice[0] > ruleChoice[1] || ruleChoice[2] > ruleChoice[3])
            {
                printf("---- Le format de la regle n'est pas conforme. ----\n");
                validChoice = 0;
            }
            else
                if((ruleChoice[1] - ruleChoice[0]) > 2 || (ruleChoice[3] - ruleChoice[2]) > 2)
                {
                    printf("---- Le format de la regle n'est pas conforme. ----\n");
                    validChoice = 0;
                }
                else
                    validChoice = 1;
    }while(!validChoice);
    strcpy(self->rule, ruleChoice);
    self->rule[4] = '\0';

    //S'il s'agit du premier type d'analyse
    if(self->typeOfAnalysis == 1)
    {
        //On demande à l'utilisateur sur quelle seed il souhaite faire son analyse
        char seedChoice[20];
        do{
            strcpy(seedChoice, "");
            printf("************************************\n");
            printf("#### Quelle seed choisissez-vous? ####\n");
            scanf("%s", seedChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(seedChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour la seed n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
                validChoice = 1;
        }while(!validChoice);
        int seed = atoi(seedChoice);
        self->seedInfRange = seed;
        self->seedSupRange = seed;

        //On demande à l'utilisateur quelle taille de cube il souhaite pour son analyse
        char gridChoice[20];
        do{
            strcpy(gridChoice, "");
            printf("************************************\n");
            printf("#### Quelle taille de grille choisissez-vous? Elle doit etre au minimum egale a 3 et superieure ou egale au nombre de processus choisi moins le processus 0 (%d)...\nLa taille correspond a la longueur d'un des cotes du cube (ex: taille 3 pour un cube de 27 cases):  ####\n", nbProc-1);
            scanf("%s", gridChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(gridChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour la taille de la grille n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
            {
                int size = atoi(gridChoice);
                if(size < 3 || size < nbProc-1)
                {
                    printf("---- La valeur que vous avez entree pour la taille de la grille n'est pas conforme. ----\n");
                    printf("---- Elle doit etre au moins egale a 3 et superieure ou egale au nombre de processus choisi moins le processus 0 (ici %d). ----\n", nbProc-1);
                    //On détruit la structure de configuration
                    Configuration__destroy(&self);
                    //Et on quitte le programme
                    exit(1);
                }
                else
                {
                    //On vérifie si le nombre de coeurs est suffisant pour la taille du cube choisie (le programme permet à chaque coeur de pouvoir recevoir au maximum l'équivalent d'un cube de dimension 150 x 150 x 150 soit 3 375 000 cellules)
                    //On récupère la taille théorique du cube choisi par l'utilisateur
                    int theoricSize = size*size*size;
                    //On détermine le nombre de données par processus
                    int dataPerProc = theoricSize / (nbProc - 1);
                    //si la valeur dépasse 3 375 000, on prévient l'utilisateur qu'il ne peut pas continuer
                    if(dataPerProc > 3375000)
                    {
                        printf("---- Le nombre de coeurs choisi est trop faible par rapport à la taille de la grille de cellules choisie. Veuillez choisir une taille plus petite ou relancez le programme...\n");
                        validChoice = 0;
                    }
                    //Sinon le résultat est valide
                    else
                        validChoice = 1;
                }
            }               
        }while(!validChoice);
        int grid = atoi(gridChoice);
        self->gridSizeInfRange = grid;
        self->gridSizeSupRange = grid;

        //Enfin on demande le nombre de générations maximale que l'utilisateur souhaite pour l'analyse
        char genChoice[20];
        do{
            strcpy(genChoice, "");
            printf("************************************\n");
            printf("#### Quelle nombre de generations maximale choisissez-vous pour l'analyse? ####\n");
            scanf("%s", genChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(genChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour le nombre de generations n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
                validChoice = 1;
        }while(!validChoice);
        self->nbGenerations = atoi(genChoice); 
    }
    //Sinon il s'agit du deuxième type d'analyse et on peut tout configurer
    else
    {
        //On demande à l'utilisateur sur quelle borne inférieure de la plage de seed il souhaite faire son analyse
        char seedInfChoice[20];
        do{
            strcpy(seedInfChoice, "");
            printf("************************************\n");
            printf("#### Selectionnez la borne inferieure de la plage de seed sur laquelle vous souhaitez faire votre analyse? ####\n");
            scanf("%s", seedInfChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(seedInfChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour la seed n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
                validChoice = 1;
        }while(!validChoice);
        self->seedInfRange = atoi(seedInfChoice);

        //Puis on demande à l'utilisateur la valeur de la borne supérieure qui doit être supérieure ou égale à celle de la borne inférieure
        char seedSupChoice[20];
        do{
            strcpy(seedSupChoice, "");
            printf("************************************\n");
            printf("#### Selectionnez la borne superieure de la plage de seed sur laquelle vous souhaitez faire votre analyse? ####\nNote: la valeur de la borne superieure doit etre superieure ou egale a celle de la borne inferieure.\n");
            scanf("%s", seedSupChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(seedSupChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour la seed n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
            {
                int choice = atoi(seedSupChoice);
                if(choice < self->seedInfRange)
                {
                    printf("---- La valeur de la borne superieure ne peut pas etre inferieure a celle de la borne inferieure... ----\n");
                    validChoice = 0;
                }
                else
                    validChoice = 1;
            }
        }while(!validChoice);
        self->seedSupRange = atoi(seedSupChoice);
        
        //Et on fait de même pour la taille de la grille avec la même condition entre la borne inférieure et la borne supérieure
        char gridSizeInfChoice[20];
        do{
            strcpy(gridSizeInfChoice, "");
            printf("************************************\n");
            printf("#### Selectionnez la borne inferieure de la plage de la taille de la grille sur laquelle vous souhaitez faire votre analyse?\n Elle doit etre au moins egale a 3 et superieure ou egale au nombre de processus choisi moins le processus 0 (%d)...\nLa taille correspond a la longueur d'un des cotes du cube (ex: taille 3 pour un cube de 27 cases): ####\n", nbProc-1);
            scanf("%s", gridSizeInfChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(gridSizeInfChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour la taille de la grille n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
            {
                int size = atoi(gridSizeInfChoice);
                if(size < 3 || size < nbProc-1)
                {
                    printf("---- La valeur que vous avez entree pour la taille de la grille n'est pas conforme. ----\n");
                    printf("---- Elle doit etre au moins egale a 3 et superieure ou egale au nombre de processus choisi moins le processus 0 (ici %d). ----\n", nbProc-1);
                    //On détruit la structure de configuration
                    Configuration__destroy(&self);
                    //Et on quitte le programme
                    exit(1);
                }
                else
                {
                    //On vérifie si le nombre de coeurs est suffisant pour la taille du cube choisie (le programme permet à chaque coeur de pouvoir recevoir au maximum l'équivalent d'un cube de dimension 150 x 150 x 150 soit 3 375 000 cellules)
                    //On récupère la taille théorique du cube choisi par l'utilisateur
                    int theoricSize = size*size*size;
                    //On détermine le nombre de données par processus
                    int dataPerProc = theoricSize / (nbProc - 1);
                    //si la valeur dépasse 3 375 000, on prévient l'utilisateur qu'il ne peut pas continuer
                    if(dataPerProc > 3375000)
                    {
                        printf("---- Le nombre de coeurs choisi est trop faible par rapport à la taille de la grille de cellules choisie. Veuillez choisir une taille plus petite ou relancez le programme...\n");
                        validChoice = 0;
                    }
                    //Sinon le résultat est valide
                    else
                        validChoice = 1;
                }
            }
        }while(!validChoice);
        self->gridSizeInfRange = atoi(gridSizeInfChoice);

        //Puis on demande à l'utilisateur la valeur de la borne supérieure qui doit être supérieure ou égale à celle de la borne inférieure
        char gridSizeSupChoice[20];
        do{
            strcpy(gridSizeSupChoice, "");
            printf("************************************\n");
            printf("#### Selectionnez la borne superieure de la plage de de la taille de la grille sur laquelle vous souhaitez faire votre analyse? ####\nNote: la valeur de la borne superieure doit etre superieure ou egale a celle de la borne inferieure.\n");
            scanf("%s", gridSizeSupChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(gridSizeSupChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour la taille de la grille n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
            {
                int choice = atoi(gridSizeSupChoice);
                if(choice < self->gridSizeInfRange)
                {
                    printf("---- La valeur de la borne superieure ne peut pas etre inferieure a celle de la borne inferieure... ----\n");
                    validChoice = 0;
                }
                else
                    validChoice = 1;
            }
        }while(!validChoice);
        self->gridSizeSupRange = atoi(gridSizeSupChoice); 

        //Enfin on demande le nombre de générations maximale que l'utilisateur souhaite pour l'analyse
        char genChoice[20];
        do{
            strcpy(genChoice, "");
            printf("************************************\n");
            printf("#### Quelle nombre de generations maximale pour une seed et une taille de grille donnees choisissez-vous pour l'analyse? ####\n");
            scanf("%s", genChoice);
            //On vérifie si le choix de l'utilisateur est un nombre
            int isNumber = Configuration__isANumber(genChoice);

            if(!isNumber)
            {
                printf("---- La valeur que vous avez entre pour le nombre de generations n'est pas un nombre. ----\n");
                validChoice = 0;
            }
            else
                validChoice = 1;
        }while(!validChoice);
        self->nbGenerations = atoi(genChoice);
    }
}

int Configuration__isANumber(char* text)
{
    //On récupère la taille du texte
    int gridSize = (int)strlen(text);
    //itérateur sur les caractères de la chaine
    int i = 0;
    //booléen pour savoir si le texte est un nombre
    int isNumber = 1;

    while(i < gridSize && isNumber)
    {
        if(!isdigit(text[i]))
            isNumber = 0;
        else
            i++;
    }

    return isNumber;
}

void Configuration__setTasksPerProc(Configuration* self, int gridSize, int nbProc)
{
    //Si la mémoire pour le tableau tasksPerProc n'a pas été allouée, alors on fait une allocation dans le tas en fonction du nombre de processus
    if(self->tasksPerProc == NULL)
        self->tasksPerProc = (int*)malloc((size_t)(nbProc-1)*sizeof(int));

    Configuration__resetNumOfTasks(self, nbProc);
    
    //On détermine le nombre de feuillets qu'auront à traiter chaque processus
    for(int i = 0; i < gridSize; i++)
    {
        self->tasksPerProc[i%(nbProc-1)]++;
    }

    //auxquels on ajoute les feuillets voisins (le processus 0 aura le sommet de la grille donc 1 feuillet voisin,
    //le dernier processus aura le fond de la grille donc 1 feuillet voisin
    //et les autres auront le feuillet du dessus et celui du dessous)
    //UNIQUEMENT S'IL Y A AU MOINS 2 processus QUI TRAVAILLENT EN PARALLELE
    if((nbProc - 1) > 1)
    {
        for(int i = 0; i < nbProc - 1; i++)
        {
            //S'il s'agit du premier ou du dernier processus
            if(i == 0 || i == (nbProc - 2))
                self->tasksPerProc[i]++;
            //Sinon pour tout autre processus autre que le premier et le dernier processus
            else
                self->tasksPerProc[i]+=2;
        }
    }
}

void Configuration__resetNumOfTasks(Configuration* self, int nbProc)
{
    for(int i = 0; i < nbProc - 1; i++)
    {
        self->tasksPerProc[i] = 0;
    }
}

void Configuration__destroy(Configuration** self)
{
    if((*self)->tasksPerProc != NULL)
    {
        free((*self)->tasksPerProc);
        (*self)->tasksPerProc = NULL;
    }
    free(*self);
    *self = NULL;
}