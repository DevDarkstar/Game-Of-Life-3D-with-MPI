#ifndef FILE_RESULTS_H
#define FILE_RESULTS_H

/**
* \file file_results.h
* \brief Gestion du stockage des résultats du programme
* \author Richard Leestmans
* \version 0.1
* \date 11 Décembre 2022
*
* Partie du programme permettant de centraliser les résultats et de générer un affichage dans une page html voire au format pdf (au choix de l'utilisateur)
* afin de permettre une meilleure interprétation des résultats.
*/

#include "configuration.h"
#include "grid.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
* \struct FileResults
* \brief Objet regroupant les informations de la création du fichier de résultats.
*
* Objet regroupant les informations sur le fichier de résultats (chemin et nom du fichier ainsi que l'adresse du fichier pour pouvoir ouvrir, fermer et stocker des données dans ce fichier).
*/
typedef struct
{
    char path[50]; /*!< Chemin du fichier de résultats*/
    FILE* file; /*!< Adresse du fichier permettant de l'ouvrir, d'y stocker des données et de le fermer*/
}FileResults;

/**
* \fn FileResults* FileResults__create(Configuration* config)
* \brief Constructeur de l'objet FileResults permettant de mettre en place la base du fichier de résultats
* ainsi que la création du dossier dans lequel le fichier de résultats sera stocké.
* 
* \param config Instance d'un objet Configuration permettant de créer le fichier de résultats en fonction de la configuration du programme choisie par l'utilisateur
* \return Instance nouvelle allouée d'un objet de type FileResults
*/
FileResults* FileResults__create(Configuration* config);
/**
* \fn void FileResults__storePattern(FileResults* self, char*** tab, int gridSize)
* \brief Fonction permettant (uniquement dans le cas d'une analyse sur une seule seed et une seule taille de grille) d'écrire dans le fichier de résultats
* une grille de cellules sous la forme de fauillets (appelés niveaux dans le fichier)
* De plus, seuls les feuillets possédant au moins ue cellule vivante seront affichés dans le fichier
* 
* \param self Instance d'un objet FileResults référençant le fichier dans lequel les données vont être écrites
* \param tab Grille de cellules à écrire dans le fichier de résultats
* \param gridSize Taille de la grille, c'est-à-dire la longueur d'un côté du cube
*/
void FileResults__storePattern(FileResults* self, char*** tab, int gridSize);
/**
* \fn void FileResults__storeResults(FileResults* self, Grid* grid, int result, int generation)
* \brief Fonction permettant (uniquement dans le cas d'une analyse sur une seule seed et une seule taille de grille) d'écrire dans le fichier de résultats
* la ou les patterns de cellules finaux en fonction des résultats de l'analyse.
* Par exemple, si l'analyse donne pour résultat une configuration oscillatoire de période 2, la fonction va écrire dans le fichier les patterns des 2 états
* de l'oscillation.
* 
* \param self Instance d'un objet FileResults référençant le fichier dans lequel les données vont être écrites
* \param grid Instance d'un objet Grid qui va contenir les patterns finaux de cellules à stocker dans le fichier
* \param result Valeur du résultat de l'analyse déterminant quoi écrire dans le fichier
* \param generation Valeur de la génération à laquelle le résultat final a été obtenu
*/
void FileResults__storeResults(FileResults* self, Grid* grid, int result, int generation);
/**
* \fn void FileResults__createResultTable(FileResults* self, Configuration* config, int* results)
* \brief Fonction permettant (uniquement dans le cas d'une analyse sur une plage de seeds et une plage de tailles de grille) d'écrire dans le fichier de résultats
* tous les résultats de toutes les analyses effectuées par le programme sous la forme d'un tableau avec un code couleur en fonction du résultat de l'analyse
* 
* \param self Instance d'un objet FileResults référençant le fichier dans lequel les données vont être écrites
* \param config Instance d'un objet Configuration contenant les informations de plage de seeds et de tailles de grilles nécessaires à la création du tableau de résultats
* \param results Tableau de tous les résultats obtenus lors de toutes les analyses effectuées
*/
void FileResults__createResultTable(FileResults* self, Configuration* config, int* results);
/**
* \fn void FileResults__exportResults(FileResults* self, Configuration* config)
* \brief Fonction permettant de finaliser le fichier de résultat et de proposer à l'utilisateur d'exporter ses résultats sous la forme d'un fichier pdf.
* L'exportation dépendra de la taille des données dans le fichier de résultats.
* 
* \param self Instance d'un objet FileResults référençant le fichier dans lequel les données vont être écrites
* \param config Instance d'un objet Configuration permettant de déterminer si l'utilisateur peut exporter les résultats sous la forme d'un fichier pdf
*/
void FileResults__exportResults(FileResults* self, Configuration* config);
/**
* \fn void FileResults__destroy(FileResults** self)
* \brief Destructeur d'une instance d'un objet FileResults.
* 
* \param self Adresse d'une instance d'un objet FileResults à détruire
*/
void FileResults__destroy(FileResults** self);

#endif