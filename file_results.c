#include "file_results.h"

FileResults* FileResults__create(Configuration* config)
{
    //On commence par créer le dossier où enregistrer les fichiers de résultats
    DIR* directory = opendir("results");
    //Si le dossier existe déjà
    if(directory)
        closedir(directory);
    //Sinon on le crée
    else
        mkdir("results", 0700);

    FileResults* results = NULL;
    results = (FileResults*)malloc(sizeof(FileResults));

    //puis on crée le chemin en fonction de la configuration choisie par l'utilisateur
    if(config->typeOfAnalysis == 1)
    {
        //On crée le nom du fichier de résultats
        sprintf(results->path, "results/seed_%d_gridSize_%d.html", config->seedInfRange, config->gridSizeInfRange);
        //On crée s'il n'existe pas déjà et on l'ouvre en mode écriture
        results->file = fopen(results->path, "w");
        //Si le fichier n'a pas pu être ouvert
        if(results->file == NULL)
        {
            perror("Impossible d'ouvrir le fichier...\n");
            exit(1);
        }
        //On écrit les spécificités de la configuration choisie par l'utilisateur
        fputs("<!DOCTYPE html><html><head>", results->file);
        fputs("<meta http-equiv='Content-Security-Policy' content='sandbox'></head><body><div>", results->file);
        fprintf(results->file, "<h2 style='text-align:center;'>PATTERN SEED %d - TAILLE DE GRILLE %d</h2>", config->seedInfRange, config->gridSizeInfRange);
        fprintf(results->file, "<h3>CONFIGURATION</h3><ul style='background-color:bisque;'><li>Type d'analyse: <strong>analyse d'une seed et d'une taille de grille précise</strong></li><li>Modèle utilisé: <strong>%s</strong></li><li>Valeur de la seed: <strong>%d</strong></li>", config->rule, config->seedInfRange);
        fprintf(results->file, "<li>Taille de la grille: <strong>%d</strong></li><li>Nombre maximale de générations: <strong>%d</strong></li></ul>", config->gridSizeInfRange, config->nbGenerations);
        fputs("<h2 style='text-align:center;'>Pattern initial</h2>", results->file);

        fclose(results->file);
    }
    else
    {
        sprintf(results->path, "results/seed_%d_to_%d_gridSize_%d_to_%d.html", config->seedInfRange, config->seedSupRange, config->gridSizeInfRange, config->gridSizeSupRange);
        //On crée s'il n'existe pas déjà et on l'ouvre en mode écriture
        results->file = fopen(results->path, "w");
        //Si le fichier n'a pas pu être ouvert
        if(results->file == NULL)
        {
            perror("Impossible d'ouvrir le fichier...\n");
            exit(1);
        }
        //On écrit les spécificités de la configuration choisie par l'utilisateur
        fputs("<!DOCTYPE html><html><head>", results->file);
        fputs("<meta http-equiv='Content-Security-Policy' content='sandbox'></head><body><div>", results->file);
        fprintf( results->file, "<h2 style='text-align:center;'>PLAGE DE DONNEES: SEED DE %d A %d ET TAILLE DE LA GRILLE DE %d A %d</h2>", config->seedInfRange, config->seedSupRange, config->gridSizeInfRange, config->gridSizeSupRange);
        fprintf(results->file, "<h3>CONFIGURATION</h3><ul style='background-color:bisque;'><li>Type d'analyse: <strong>analyse sur une plage de seed et une plage de taille de grille</strong></li><li>Modèle utilisé: <strong>%s</strong></li><li>Valeur de la borne inférieure de la seed: <strong>%d</strong></li><li>Valeur de la borne supérieure de la seed: <strong>%d</strong></li>", config->rule, config->seedInfRange, config->seedSupRange);
        fprintf(results->file, "<li>Valeur de la borne inférieure de la taille de la grille: <strong>%d</strong></li><li>Valeur de la borne supérieure de la taille de la grille: <strong>%d</strong></li><li>Nombre de génération maximale par cycle: <strong>%d</strong></li></ul>", config->gridSizeInfRange, config->gridSizeSupRange, config->nbGenerations);

        fclose(results->file);
    }

    return results;
}

void FileResults__storePattern(FileResults* self, char*** tab, int gridSize)
{
    self->file = fopen(self->path, "a");
    if(self->file == NULL)
    {
        perror("Impossible d'ouvrir le fichier...\n");
        exit(1);
    }
    //char buffer[(size_t)(gridSize*gridSize*gridSize + 500)];
    for(int i = 0; i < gridSize; i++)
    {
         //On vérifie que le niveau ou feuillet n'est pas vide afin de ne garder dans le fichier de sortie que les zones d'intérêt
        if(!Grid__levelIsEmpty(tab[i], gridSize))
        {
            fprintf(self->file, "<p style='font-weight:bold;text-align:center;background-color:bisque;border-bottom:solid 2px;'>NIVEAU %d</p>", i);
            fputs("<p style='text-align:center;font-weight:bold;background-color:bisque;'>", self->file);
            for(int j = 0; j < gridSize; j++)
            {        
                char bufferRow[gridSize + 1];
                for(int k = 0; k < gridSize; k++)
                {
                    if(tab[i][j][k] == '0')
                        bufferRow[k] = '_';
                    else
                        bufferRow[k] = '1';
                }
                bufferRow[gridSize] = '\0';
                fprintf(self->file, bufferRow);
                fputs("<br/>", self->file);
            }
            fprintf(self->file, "</p><br/>");
            //fputs(buffer, self->file);
        }
    }

    fclose(self->file);
}

void FileResults__storeResults(FileResults* self, Grid* grid, int result, int generation)
{
    self->file = fopen(self->path, "a");
    if(self->file == NULL)
    {
        perror("Impossible d'ouvrir le fichier...\n");
        exit(1);
    }
    //Selon la valeur du résultat, on écrit un certain nombre de pattern dans le fichier de sortie
    switch(result)
    {
        //Cas où il n'y a pas de configuratons stables trouvées car il n'y a plus de cellules vivantes
        case 0:
            fputs("<p style='font-weight:bold;'>Pas de configuration stable trouvee pour ce pattern car il n'y avait plus de cellules viavntes :(</p>", self->file);
            fclose(self->file);
            break;
        //Cas où il y a une configuration stable (l'adresse du pattern est contenue dans la variable NGeneration de la structure grid)
        case 1:
            fprintf(self->file, "<h3 style='text-align:center;'>Configuration stable (Génération %d)</h3>", generation);
            fclose(self->file);
            FileResults__storePattern(self, grid->NGeneration, grid->gridSize);
            break;
        //Cas où il y a une configuration oscillatoire de période 2 (l'adresse du pattern de l'état 1 est contenue dans NMinusOneGeneration et celle de l'état 2 dans NGeneration)
        case 2:
            fprintf(self->file, "<h3 style='text-align:center;'>Configuration stable oscillatoire de période 2 - état 1 (Génération %d)</h3>", generation);
            fclose(self->file);
            FileResults__storePattern(self, grid->NMinusOneGeneration, grid->gridSize);
            self->file = fopen(self->path, "a");
            if(self->file == NULL)
            {
                perror("Impossible d'ouvrir le fichier...\n");
                exit(1);
            }
            fprintf(self->file, "<h3 style='text-align:center;'>Configuration stable oscillatoire de période 2 - état 2 (Génération %d)</h3>", generation + 1);
            fclose(self->file);
            FileResults__storePattern(self, grid->NGeneration, grid->gridSize);
            break;
        //Cas où il y a une configuration oscillatoire de période 2 (l'adresse du pattern de l'état 1 est contenue dans NMinusTwoGeneration, celle de l'état 2 dans NMinusOneGeneration, celle de l'état 3 dans NGeneration) 
        case 3:
            fprintf(self->file, "<h3 style='text-align:center;'>Configuration stable oscillatoire de période 3 - état 1 (Génération %d)</h3>", generation);
            fclose(self->file);
            FileResults__storePattern(self, grid->NMinusTwoGeneration, grid->gridSize);
            self->file = fopen(self->path, "a");
            if(self->file == NULL)
            {
                perror("Impossible d'ouvrir le fichier...\n");
                exit(1);
            }
            fprintf(self->file, "<h3 style='text-align:center;'>Configuration stable oscillatoire de période 3 - état 2 (Génération %d)</h3>", generation + 1);
            fclose(self->file);
            FileResults__storePattern(self, grid->NMinusOneGeneration, grid->gridSize);
            self->file = fopen(self->path, "a");
            if(self->file == NULL)
            {
                perror("Impossible d'ouvrir le fichier...\n");
                exit(1);
            }
            fprintf(self->file, "<h3 style='text-align:center;'>Configuration stable oscillatoire de période 3 - état 3 (Génération %d)</h3>", generation + 2);
            fclose(self->file);
            FileResults__storePattern(self, grid->NGeneration, grid->gridSize);
            break;
        //Cas où il n'y a pas de configurations stables trouvées pour le nombre de générations choisi
        default:
            fprintf(self->file, "<p style='font-weight:bold;'>Pas de configuration stable trouvee en %d générations pour ce pattern :(</p>", grid->nbGenerations);
            fprintf(self->file, "<h3 style='text-align:center;'>Pattern de la génération %d</h3>", grid->nbGenerations);
            fclose(self->file);
            FileResults__storePattern(self, grid->NGeneration, grid->gridSize);
    }
}

void FileResults__createResultTable(FileResults* self, Configuration* config, int* results)
{
    self->file = fopen(self->path, "a");
    if(self->file == NULL)
    {
        perror("Impossible d'ouvrir le fichier...\n");
        exit(1);
    }
    //On commence par créer la légende du tableau
    fputs("<div style='width:800px;height:20px;font-weight:bold;border:solid;'>Pas de configuration stable trouvée car plus de cellules vivantes</div><br/>", self->file);
    fputs("<div style='width:800px;height:20px;background-color:lightgrey;font-weight:bold;'>Pas de configuration stable trouvée pour le nombre de générations choisi</div><br/>", self->file);
    fputs("<div style='width:800px;height:20px;background-color:darksalmon;font-weight:bold;'>Configuration stable</div><br/>", self->file);
    fputs("<div style='width:800px;height:20px;background-color:aqua;font-weight:bold;'>Configuration oscillatoire de période 2</div><br/>", self->file);
    fputs("<div style='width:800px;height:20px;background-color:aquamarine;font-weight:bold;'>Configuration oscillatoire de période 3</div><br/>", self->file);
    fputs("<div style='margin:auto;'><table style='border:solid;border-collapse:collapse;font-weight:bold;'><thead><tr><th style='border:solid;'>Seed/Taille de la grille</th>", self->file);
    for(int i = config->gridSizeInfRange; i <= config->gridSizeSupRange; i++)
    {
        fprintf(self->file, "<th style='border:solid;'>Taille %d</th>", i);
    }
    fputs("</tr></thead><tbody>", self->file);
    //On récupère 
    int diffSeed = config->seedSupRange - config->seedInfRange + 1;
    int diffSize = config->gridSizeSupRange - config->gridSizeInfRange + 1;
    for(int i = 0; i < diffSeed; i++)
    {
        fprintf(self->file, "<tr><td style='border:solid;text-align:center;font-weight:bold;'>%d</td>", i + config->seedInfRange);
        //On remplit le tableau de résultats en coloriant chaque case de ce dernier en fonction du résultat de l'analyse obtenu
        for(int j = 0; j < diffSize; j++)
        {
            switch(results[j*diffSeed + i])
            {
                case 1:
                    fputs("<td style='border:solid;background-color:darksalmon;'></td>", self->file);
                    break;
                case 2:
                    fputs("<td style='border:solid;background-color:aqua;'></td>", self->file);
                    break;
                case 3:
                    fputs("<td style='border:solid;background-color:aquamarine;'></td>", self->file);
                    break;
                case 4:
                    fputs("<td style='border:solid;background-color:lightgrey;'></td>", self->file);
                    break;
                default:
                    fputs("<td style='border:solid;'></td>", self->file);
            }
        }
        fputs("</tr>", self->file);
    }
    fputs("</tbody></table></div>", self->file);

    fclose(self->file);
}

void FileResults__exportResults(FileResults* self, Configuration* config)
{
    self->file = fopen(self->path, "a");
    if(self->file == NULL)
    {
        perror("Impossible d'ouvrir le fichier...\n");
        exit(1);
    }
    //On termine l'arborescence du fichier html
    fputs("</div></body></html>", self->file);
    fclose(self->file);
    //On récupère la longueur de l'intervalle de la plage de tailles de grille dans le cas de l'analyse 2
    int diffSize = config->gridSizeSupRange - config->gridSizeInfRange + 1;
    //On récupère la taille de la grille dans le cas de l'analyse sur une seule taille de grille de cellules
    int gridSize = config->gridSizeInfRange;
    //Si la longueur de l'intervalle est inférieure ou égale à 12
    //On demande à l'utilisateur s'il souhaite enregistrer les résultats au format pdf (dans le cas de l'analyse 1)
    if(config->typeOfAnalysis == 1)
    {
        if(gridSize <= 85)
        {
            int answerIsCorrect = 0;
            int result = 0;
            do{
                char buffer[20];
                printf("--Souhaitez-vous enregistrer les donnees au format pdf? (1) pour oui (0) pour non...--\n");
                scanf("%s", buffer);
                int answer = atoi(buffer);
                if(answer == 1)
                {
                    answerIsCorrect = 1;
                    result = 1;
                }
                else if(answer == 0)
                {
                    answerIsCorrect = 1;
                    result = 0;
                }
            }while(!answerIsCorrect);
            if(result)
            {
                //On convertit le fichier html en fichier pdf grâce à Libreoffice (nécessite Libreoffice sur la machine pour pouvoir utiliser les lignes de commandes suivantes)
                char export[200];
                sprintf(export, "soffice --headless --convert-to pdf:writer_pdf_Export %s --outdir results/", self->path);
                system(export);
            }
        }
    }
    //sinon pour une analyse de type 2
    else
    {
        if(diffSize <= 12)
        {
            int answerIsCorrect = 0;
            int result = 0;
            do{
                char buffer[20];
                printf("--Souhaitez-vous enregistrer les donnees au format pdf? (1) pour oui (0) pour non...--\n");
                scanf("%s", buffer);
                int answer = atoi(buffer);
                if(answer == 1)
                {
                    answerIsCorrect = 1;
                    result = 1;
                }
                else if(answer == 0)
                {
                    answerIsCorrect = 1;
                    result = 0;
                }
            }while(!answerIsCorrect);
            if(result)
            {
                //On convertit le fichier html en fichier pdf grâce à Libreoffice (nécessite Libreoffice sur la machine pour pouvoir utiliser les lignes de commandes suivantes)
                char export[200];
                sprintf(export, "soffice --headless --convert-to pdf:writer_pdf_Export %s --outdir results/", self->path);
                system(export);
            }
        }
    }
    //Et on libère la mémoire allouée à la création du fichier de résultats
    FileResults__destroy(&self);
}

void FileResults__destroy(FileResults** self)
{
    free(*self);
    *self = NULL;
}