#include "grid.h"
#include "configuration.h"
#include "file_results.h"
#include "data.h"
#include "mpi.h"
#include <time.h>

//Compilation
// gcc -std=c11 -Wall -Wextra -Werror -Wpedantic -Wconversion main.c grid.c configuration.c file_results.c data.c -o main
int main(int argc, char** argv)
{
    //Initialisation de MPI
    MPI_Init (&argc ,&argv);

    int numprocs, rank;
    //On récupère le nombre de processus choisi par l'utilisateur
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    //On récupère le rang du processus qui gère ce fichier
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //On crée un objet status pour les MPI_Recv
    MPI_Status stat;
    //Et un booléen gérant la condition d'arrêt de tous les processus autres que le processus 0
    int hasFinished = 0;

    //S'il s'agit du processus 0
    if(rank == 0)
    {
        Configuration* config = NULL;
        //Lorsqu'aucune option n'a été renseignée, on lance la création de la configuration personnalisée du jeu de la vie en passant le nombre de processus indiqués par l'utilisateur au moement de l'exécution du programme
        if(argv[1] == NULL)      
            config = Configuration__create(numprocs);
        //Sinon l'utilisateur a rentré un paramètre. Par conséquent, il souhaite lancer un test
        else
        {
            //sur une analyse simple (analyse sur une seed et une taille de grille définies)
            if(strcmp(argv[1], "test1") == 0)
                config = Configuration__createSingleAnalysis(numprocs);
            //sur une plage de données de seed et de taille de grille définies
            else if(strcmp(argv[1], "test2") == 0)
                config = Configuration__createDataRangeAnalysis(numprocs);
            //sinon, le paramètre indiqué est incorrect
            else
            {
                printf("Une erreur s'est produite due a une erreur sur l'option choisie dans la commande d'execution du programme...\n");
                exit(1);
            }
        }
        printf("Les parametres enregistres sont: type d'analyse: %d, regle: %s, seedInf: %d, seedSup: %d, taille de la grille Inf: %d, taille de la grille Sup: %d, nombre de generations: %d\n", config->typeOfAnalysis, config->rule, config->seedInfRange, config->seedSupRange, config->gridSizeInfRange, config->gridSizeSupRange, config->nbGenerations);
        FileResults* file_results = FileResults__create(config);
        time_t start, end;
        double duration;

        start = clock();

        Grid* grid = Grid__create(config->typeOfAnalysis, config->rule, config->nbGenerations, numprocs);
        //On récupère la longueur de l'intervalle entre la borne inférieure et la borne supérieure de la plage de seeds
        int diffSeed = config->seedSupRange - config->seedInfRange + 1;
        //ainsi que la longueur de l'intervalle entre la borne inférieure et la borne supérieure de la plage de tailles de grilles
        int diffSize = config->gridSizeSupRange - config->gridSizeInfRange + 1;
        //On crée le tableau de résultats en fonction de la longueur de ces deux intervalles
        //Dans le cas de l'analyse d'une seed et d'une taille de grille, ce tableau contiendra donc une valeur
        int tabResults [diffSize * diffSeed];
        //compteur sur les résultats obtenus
        int nbResults = 0;
        for(int i = config->gridSizeInfRange; i <= config->gridSizeSupRange; i++)
        {
            //On met à jour la valeur de la taille de la grille
            Grid__setGridSize(grid, i);
            //On définit le nombre de tâches que va avoir chaque processus, c'est-à-dire ici le nombre de feuillets à analyser par processus
            Configuration__setTasksPerProc(config, i, numprocs);
            for(int k = 0; k < numprocs - 1; k++)
            {
                printf("nombre de feuillets envoyes au processus %d: %d\n", k+1, config->tasksPerProc[k]);
            }
            printf("--------------------------------------------------------------------------------\n");
            Grid__setTasksPerProc(grid, config->tasksPerProc);
            for(int j = config->seedInfRange; j <= config->seedSupRange; j++)
            {
                srand((unsigned)j);
                Grid__fillGrid(grid);
                //1ère génération de cellules
                if(config->typeOfAnalysis == 1)
                    FileResults__storePattern(file_results, grid->NGeneration, grid->gridSize);
                //On récupère le résultat de l'analyse
                int* results = NULL;
                results = Grid__analyzePattern(grid);
                
                switch(results[0])
                {
                    case 0:
                        printf("--------------------------------------------------------------------------------\n");
                        printf("+ Pas de configuration stable trouvee pour un cube de taille %d (c'est-a-dire un cube de %ld cellules) et de seed %d car il n'y avait plus de cellules vivantes a la generation %d.\n\n", i, (size_t)(i*i*i), j, results[1]);
                        break;
                    case 1:
                        printf("--------------------------------------------------------------------------------\n");
                        printf("+ Une configuration stable a ete trouvee a la generation %d! Pour un cube de taille %d (c'est-a-dire un cube de %ld cellules) et de seed %d.\n\n", results[1], i, (size_t)(i*i*i), j);
                        break;
                    case 2:
                        printf("--------------------------------------------------------------------------------\n");
                        printf("+ Une configuration oscillatoire de periode 2 a ete trouvee a la generation %d! Pour un cube de taille %d (c'est-a-dire un cube de %ld cellules) et de seed %d.\n\n", results[1], i, (size_t)(i*i*i), j);
                        break;
                    case 3:
                        printf("--------------------------------------------------------------------------------\n");
                        printf("+ Une configuration oscillatoire de periode 3 a ete trouvee a la generation %d! Pour un cube de taille %d (c'est-a-dire un cube de %ld cellules) et de seed %d.\n\n", results[1], i, (size_t)(i*i*i), j);
                        break;
                    default:
                        printf("--------------------------------------------------------------------------------\n");
                        printf("+ Pas de configuration stable trouvee en %d generations pour un cube de taille %d (c'est-a-dire un cube de %ld cellules) et de seed %d.\n\n", results[1], i, (size_t)(i*i*i), j);
                }
                                   
                if(config->typeOfAnalysis == 1)
                    FileResults__storeResults(file_results, grid, results[0], results[1]);
                else
                {
                    tabResults[nbResults] = results[0];
                    nbResults++;
                }
                free(results);
                results = NULL;
                Grid__setToDefault(grid);
            }
        }
        if(config->typeOfAnalysis == 2)
            FileResults__createResultTable(file_results, config, tabResults);
        Grid__destroy(&grid);

        //Comme on a terminé l'exécution des tâches parallélisables, on demande au processus 0 d'envoyer un message d'arrêt aux autres processus
        //Pour cela, ce dernier va envoyer le message "arret" afin de permettre aux autres processus de sortir de leur boucle infinie
        char message[] = "arret";
        for(int i = 0; i < numprocs - 1; i++)
        {
            MPI_Send(message, 6, MPI_BYTE, i+1, 0, MPI_COMM_WORLD);
        }

        FileResults__exportResults(file_results, config);
        Configuration__destroy(&config);
        
        printf("travail termine pour le processus %d\n", rank);
        end = clock();
        duration = ((double)end - (double)start)/CLOCKS_PER_SEC;
        printf("\nDuree d'execution du programme %.04f secondes.\n", duration);
    }
    //Sinon, il s'agit d'un autre processus
    else
    {
        while(!hasFinished)
        {
            char receivedData[3375001];
            //buffer -> buffer pour stocker les données reçues
            //count -> nombre maximum d'éléments à recevoir
            //datatype -> type de données à recevoir
            //source -> rang du processus qui a envoyé les données
            //tag -> tag du message (entier)
            //comm -> communicateur utilisé pour la reception des données
            //status -> le status de l'objet
            MPI_Recv(receivedData, 3375020, MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if(strcmp(receivedData, "arret") == 0)
            {
                hasFinished = 1;
                printf("travail termine pour le processus %d\n", rank);
            }
            else
            {
                //On crée une nouvelle instance de la structure Data pour stocker les données reçues par le processus 0
                Data* data = NULL;
                data = Data__create(receivedData);
                char* newData = NULL;
                newData = Data__setNewGen(data, rank, numprocs);
                //On crée un buffer qui va stocker les données de la nouvelle génération à envoyer au processus 0
                //On récupère la taille des données à envoyer
                int bufferSize = (int)strlen(newData) + 1;
                char buffer[bufferSize];
                //Et on copie les données à envoyer dans ce buffer
                strcpy(buffer, newData);
                //printf("donnees envoyees au processus 0 par le processus %d:\n%s\n", rank, buffer);
                //buffer -> données à envoyer
                //count -> nombre d'éléments à envoyer
                //datatype -> type de données à envoyer
                //dest -> rang du processus auquel envoyer les données
                //tag -> tag du message (entier)
                //comm -> communicateur utilisé pour l'envoi des données
                MPI_Send(buffer, bufferSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
                //On libère la mémoire allouée aux données pointées par la variable newData
                free(newData);
                newData = NULL;
                //ainsi que la structure de données
                Data__destroy(&data);
            }
        }
    }
    //On termine l'utilisation de MPI
    MPI_Finalize();

    return 0;
}