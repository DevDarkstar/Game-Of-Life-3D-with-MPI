#ifndef GRID_H
#define GRID_H

/**
* \file grid.h
* \brief Gestion de tout ce qui touche aux générations de cellules géré par le processus 0
* \author Richard Leestmans
* \version 0.1
* \date 11 Décembre 2022
*
* Partie du programme de tout ce qui touche à la gestion des générations de cellules effectuée par le processus 0.
* On va notamment pouvoir y générer la première génération de cellules pour une seed donnée, découper la grille de cellules en multiples fragments à envoyer à tous les autres processus,
* récupérer les résultats de ces processus, reconstruire une grille à partir des données récupérées, comparer cette grille avec les grilles des anciennes générations, et obtenir un résultat 
* en fonction de ces comparaisons.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* \struct Grid
* \brief Objet regroupant les informations de gestion des différentes générations, et tout ce qui y est liée, par le processus 0.
*
* Objet regroupant toutes les informations permettant au processus 0 de définir le nombre de plans (feuillets) à envoyer à chaque processus à partir de la génération actuelle, de récupérer les résultats des autres processus,
* de les interpréter, et de les comparer avec des générations antérieures gardées en mémoire par le processus 0.
*/
typedef struct
{
    char*** NGeneration; /*!< Génération actuelle de cellules*/
    char*** NMinusOneGeneration; /*!< Avant-dernière génération de cellules*/
    char*** NMinusTwoGeneration; /*!< Antépénultième génération de cellules*/
    int gridSize; /*!< Taille de la grille, c'est-à-dire la longueur d'un des côtés du cube*/
    int typeOfAnalysis; /*!< Type de l'analyse: (1) Analyse sur une seule seed et une seule taille de grille; (2) Analyse sur une plage de seeds et une plage de tailles de grille*/
    char rule[5]; /*!< Règle choisie par l'utilisateur pour l'analyse des cellules dans la grille*/
    int nbGenerations; /*!< Nombre maximal de générations choisi par l'utilisateur sur lesquelles analyser les cellules de la grille*/
    char** receivedData; /*!< Tableau des résultats reçus par tous les autres processus du programme*/
    int nbProc; /*!< Nombre total de processus utilisés lors de l'exécution du programme*/
    int* tasksPerProc; /*!< Nombre de feuillets (c'est-à-dire les plans du cube découpés selon l'axe x) assignés à chaque processus.*/
}Grid;

/**
* \fn Grid* Grid__create(int typeOfAnalysis, char* rule, int nbGenerations, int nbProc)
* \brief Constructeur de l'objet Grid où sont initialisés les contenus des attributs de la structure
* 
* \param typeOfAnalysis Type de l'analyse
* \param rule Règle de l'analyse
* \param nbGenerations Nombre maximal de générations à effectuer lors de l'analyse
* \param nbProc Nombre total de processus utilisés lors de l'exécution du programme
* \return Instance nouvelle allouée d'un objet de type Grid
*/
Grid* Grid__create(int typeOfAnalysis, char* rule, int nbGenerations, int nbProc);
/**
* \fn int Grid__getGridSize(Grid* self)
* \brief Accesseur sur la taille de la grille contenant les cellules.
* 
* \param self Instance d'un objet Grid contenant la taille de grille à retourner
* \return Taille de la grille contenue dans l'instance de Grid
*/
int Grid__getGridSize(Grid* self);
/**
* \fn void Grid__setGridSize(Grid* self, int gridSize)
* \brief Mutateur sur la taille de la grille contenant les cellules.
* 
* \param self Instance d'un objet Grid contenant la taille de grille à modifier
* \param gridSize Nouvelle taille de grille
*/
void Grid__setGridSize(Grid* self, int gridSize);
/**
* \fn void Grid__setTasksPerProc(Grid* self, int* tasksPerProc)
* \brief Mutateur sur le nombre de plans (feuillets) à affecter à chaque processus autre que le processus 0.
* 
* \param self Instance d'un objet Grid contenant le tableau du nombre de plans de chaque processus à modifier
* \param tasksPerProc Nouveau tableau du nombre de plans à affecter à chaque processus
*/
void Grid__setTasksPerProc(Grid* self, int* tasksPerProc);
/**
* \fn int Grid__getNbGeneration(Grid* self)
* \brief Accesseur sur le nombre maximal de générations autorisées pour une analyse.
* 
* \param self Instance d'un objet Grid contenant le nombre maximal de générations 
* \return Le nombre maximal de générations contenu dans l'instance passée en paramètre
*/
int Grid__getNbGeneration(Grid* self);
/**
* \fn void Grid__fillGrid(Grid* self)
* \brief Fonction permettant de remplir la grille de cellules NGeneration en fonction de la seed utilisée.
* 
* \param self Instance d'un objet Grid contenant la grille de cellules NGeneration à remplir 
*/
void Grid__fillGrid(Grid* self);
/**
* \fn void Grid__displayGrid(char*** currentGen, int gridSize, int nbPlanes)
* \brief Fonction permettant d'afficher une génération de cellules contenue dans une grille.
* 
* \param currentGen Grille contenant la génération de cellules à afficher
* \param gridSize Taille de la grille (ici, la longueur des côtés de la grille selon les axes y et z)
* \param nbPlanes Nombre de plans de la grille (la longueur des côtés de la grille selon l'axe x) 
*/
void Grid__displayGrid(char*** currentGen, int gridSize, int nbPlanes);
/**
* \fn void Grid__setToDefault(Grid* self)
* \brief Fonction permettant de remettre à zéro le contenu des grilles de cellules NGeneration, NMinusOneGeneration et NMinusTwoGeneration.
* En d'autres termes, la fonction va libérer l'espace mémoire occupé par ces différentes grilles.
* 
* \param self Instance de Grid contenant les grilles de cellules à remettre à zéro
*/
void Grid__setToDefault(Grid* self);
/**
* \fn int Grid__compareGen(Grid* self, char*** newGen)
* \brief Fonction permettant de comparer une génération de cellules avec toutes celles stockées en mémoire dans une instance d'un objet Grid.
* 
* \param self Instance de Grid contenant les grilles de cellules des générations antérieures
* \param newGen Grille de générations de cellules à comparer avec les générations antérieures
* \return Un entier parmi les valeurs suivantes:
* 0 -> La nouvelle génération ne correspond à aucune génération stockée en mémoire;
* 1 -> La nouvelle génération correspond à la dernière génération stockée en mémoire (dans NGeneration);
* 2 -> La nouvelle génération correspond à l'avant-dernière génération stockée en mémoire (dans NMinusOneGeneration);
* 3 -> La nouvelle génération correspond à l'antépénultième génération stockée en mémoire (dans NMinusTwoGeneration).
*/
int Grid__compareGen(Grid* self, char*** newGen);
/**
* \fn int Grid__isEmpty(char*** newGen, int gridSize)
* \brief Fonction permettant de vérifier si une grille de cellules est vide, c'est-à-dire si elle ne contient que des cellules mortes.
* 
* \param newGen Grille de cellules à vérifier
* \param gridSize Taille de la grille, c'est-à-dire la longueur d'un des côtés de la grille de cellules
* \return 1 si la grille de cellules est vide 0 sinon
*/
int Grid__isEmpty(char*** newGen, int gridSize);
/**
* \fn int Grid__levelIsEmpty(char** level, int gridSize)
* \brief Fonction permettant de vérifier si un plan (ou feuillet) de cellules est vide, c'est-à-dire si elle ne contient que des cellules mortes.
* 
* \param level Feuillet de cellules à vérifier
* \param gridSize Taille de la grille, c'est-à-dire la longueur d'un des côtés de la grille de cellules (ici la longueur d'un des côtés du feuillet)
* \return 1 si la feuillet de cellules est vide 0 sinon
*/
int Grid__levelIsEmpty(char** level, int gridSize);
/**
* \fn void Grid__updatePatterns(Grid* self, char*** newGen)
* \brief Fonction permettant de mettre à jour les générations de cellules stockées en mémoire en fonction en fonction de la nouvelle passée en paramètre.
* En d'autres termes, la grille de cellules NGeneration va contenir la nouvelle génération de cellules et on va décaler toutes les générations 
* déjà présentes en mémoire d'un cran vers les générations plus antérieures.
* 
* \param self Instance d'un objet Grid contenant les grilles de cellules à mettre à jour
* \param newGen Nouvelle génération de cellules à stocker en mémoire
*/
void Grid__updatePatterns(Grid* self, char*** newGen);
/**
* \fn char** Grid__createDataPerProc(Grid* self)
* \brief Fonction qui va découper la grille contenant la génération de cellules et générer, pour chaque processus, un datagramme, sous la forme d'une chaine de caractères, contenant toutes les informations pour que le processus le recevant puisser analyser les données à l'intérieur.
* Le datagramme sera de la forme: modèle_taille de la grille_taille des données_données
* Exemple : 5766_3_18_011000100110011101
* 
* \param self Instance d'un objet Grid contenant la grille de cellules à découper
* \return un tableau contenant l'ensemble des datagrammes destinés à tous les autres processus du programme
*/
char** Grid__createDataPerProc(Grid* self);
/**
* \fn void Grid__initializeReceivedDataTab(Grid* self)
* \brief Fonction permettant d'initialiser le tableau receivedData servant à stocker dans chacun de ses buffers les résultats reçus par chaque processus.
* 
* \param self Instance d'un objet Grid contenant le tableau de résultats à initialiser
*/
void Grid__initializeReceivedDataTab(Grid* self);
/**
* \fn char*** Grid__dataToGrid(Grid* self)
* \brief Fonction permettant de transformer les résultats reçus par le processus 0 sous la forme de chaines de caractères en grille de cellules à trois dimensions.
* 
* \param self Instance d'un objet Grid contenant le tableau de résultats
* \return La grille de cellules à 3 dimensions créée à partir des résultats obtenus
*/
char*** Grid__dataToGrid(Grid* self);
/**
* \fn int* Grid__analyzePattern(Grid* self)
* \brief Fonction généraliste regroupant une grosse partie du processus du processus 0, de la création de datagrammes à partir du découpage de la grille, à la comparaison de la nouvelle génération de cellules avec
* celles déjà stockées en mémoire, en passant par la récupération des résultats des autres processus grâce à MPI et la création de la grille de la nouvelle génération à partir de ces résultats. 
* Et tout ceci pour toutes les générations d'une seed et une taille de grille donnée.
*
* \param self Instance d'un objet Grid qui va servir à réaliser tout le processus cité ci-dessus.
* \return Un tableau de résultats contenant deux valeurs. La première est un entier parmi les valeurs suivantes en fonction du résultat:
* 0 -> pas de configuration stable trouvée car plus de cellules vivantes;
* 1 -> configuration stable trouvée;
* 2 -> configuration oscillatoire de période 2;
* 3 -> configuration oscillatoire de période 3;
* 4 -> pas de configuration stable trouvée pour le nombre de générations choisi.
* La seconde valeur du tableau des résultats correspond à la génération à partir de laquelle la première valeur a été obtenue.
*/
int* Grid__analyzePattern(Grid* self);
/**
* \fn void Grid__destroy(Grid** self)
* \brief Destructeur d'une instance de Grid
* 
* \param self Adresse d'un objet de Grid à détruire
*/
void Grid__destroy(Grid** self);

#endif