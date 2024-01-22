#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/**
* \file configuration.h
* \brief Configuration du programme du jeu de la vie
* \author Richard Leestmans
* \version 0.1
* \date 11 Décembre 2022
*
* Partie du programme permettant la configuration du jeu de la vie (choix du type d'analyse, de la seed, de la taille de la grille, du nombre de générations sur lesquelles effectuer notre analyse, ...).
* Il est également possible d'effectuer une analyse sur une plage de données: plage de seeds et plage de tailles de grilles.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
* \struct Configuration
* \brief Objet regroupant les informations de configuration.
*
* Objet regroupant toutes les informations, soit entrées par l'utilisateur, soit définies automatiquement (seulement pour les tests), et permettant de mener à bien l'exécution du programme.
*/
typedef struct
{
    int typeOfAnalysis; /*!< Type de l'analyse: (1) Analyse sur une seed et une taille de grille; (2) Analyse sur une plage de seeds et de tailles de grilles.*/
    char rule[5];/*!< Règle utilisée dans le jeu de la vie de la forme X1X2Y1Y2 avec X1 et X2 les bornes inférieure et supérieure du nombre de cellules vivantes adjacentes permettant de garder une cellule vivante en vie
    et Y1 et Y2 les bornes inférieure et supérieure du nombre de cellules vivantes adjacentes permettant à une cellule morte de revenir à la vie.
    A titre d'exemple, le format pour le jeu de la vie classique en deux dimensions s'écrit 2333.*/
    int seedInfRange; /*!< Borne inférieure de la plage de seeds sur laquelle effectuer le jeu de la vie.*/
    int seedSupRange; /*!< Borne supérieure de la plage de seeds sur laquelle effectuer le jeu de la vie.
    Dans le cas d'une analyse sur une seed, les valeurs des bornes supérieure et inférieure sont identiques.*/
    int gridSizeInfRange; /*!< Borne inférieure de la plage de tailles de grille sur laquelle effectuer le jeu de la vie.
    La taille de la grille correspond à la longueur d'un des côtés du cube qui contient la génération de cellules.*/
    int gridSizeSupRange; /*!< Borne supérieure de la plage de tailles de grille sur laquelle effectuer le jeu de la vie.
    Dans le cas d'une analyse sur une taille de grille, les valeurs des bornes inférieure et supérieure sont identiques.*/
    int nbGenerations; /*!< Nombre de générations maximales sur lesquelles effectuer l'analyse d'une grille de taille et de seed donnée.*/
    int* tasksPerProc; /*!< Nombre de feuillets (c'est-à-dire les plans du cube découpés selon l'axe x) assignés à chaque processus.*/
}Configuration;

/**
* \fn Configuration* Configuration__create(int nbProc)
* \brief Constructeur de l'objet Configuration dans le cas d'une analyse où les paramètres sont choisis par l'utilisateur
* 
* \param nbProc Nombre de processus totaux utilisés lors de l'exécution du programme
* \return Instance nouvelle allouée d'un objet de type Configuration
*/
Configuration* Configuration__create(int nbProc);
/**
* \fn Configuration* Configuration__createSingleAnalysis(int nbProc)
* \brief Constructeur de l'objet Configuration dans le cas d'un test de l'analyse d'une seed et d'une taille de grille donnée
* 
* \param nbProc Nombre de processus totaux utilisés lors de l'exécution du programme
* \return Instance nouvelle allouée d'un objet de type Configuration
*/
Configuration* Configuration__createSingleAnalysis(int nbProc);
/**
* \fn Configuration* Configuration__createDataRangeAnalysis(int nbProc)
* \brief Constructeur de l'objet Configuration dans le cas d'un test de l'analyse d'une plage de seeds et d'une plage de tailles de grilles données
* 
* \param nbProc Nombre de processus totaux utilisés lors de l'exécution du programme
* \return Instance nouvelle allouée d'un objet de type Configuration
*/
Configuration* Configuration__createDataRangeAnalysis(int nbProc);

/**
* \fn void Configuration__setParameters(Configuration* self, int nbProc)
* \brief fonction permettant de définir tous les attributs d'une instance de Configuration
* 
* \param self Instance de Configuration pour laquelle définir l'ensemble de ses attributs
* \param nbProc Nombre de processus totaux utilisés lors de l'exécution du programme
*/
void Configuration__setParameters(Configuration* self, int nbProc);
/**
* \fn int Configuration__isANumber(char* text)
* \brief fonction permettant de déterminer si une chaine de caractère est un nombre
* 
* \param text Chaine de caractères à analyser
* \return 1 si la chaine de caractères est un nombre 0 sinon
*/
int Configuration__isANumber(char* text);
/**
* \fn void Configuration__setTasksPerProc(Configuration* self, int gridSize, int nbProc)
* \brief fonction permettant de définir le nombre de feuillets affectés à chaque processus
* 
* \param self Instance de l'objet Configuration
* \param gridSize Taille actuelle de la grille (longueur d'un de ses côtés)
* \param nbProc Nombre de processus totaux utilisés lors de l'exécution du programme
*/
void Configuration__setTasksPerProc(Configuration* self, int gridSize, int nbProc);
/**
* \fn void Configuration__resetNumOfTasks(Configuration* self, int nbProc)
* \brief fonction permettant de remettre à zéro le nombre de feuillets assignés à chaque processus
* 
* \param self Instance de l'objet Configuration
* \param nbProc Nombre de processus totaux utilisés lors de l'exécution du programme
*/
void Configuration__resetNumOfTasks(Configuration* self, int nbProc);
/**
* \fn void Configuration__destroy(Configuration** self)
* \brief destructeur d'une instance de l'objet Configuration
* 
* \param self Adresse de l'instance de Configuration à détruire
*/
void Configuration__destroy(Configuration** self);

#endif