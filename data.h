#ifndef DATA_H
#define DATA_H

/**
* \file data.h
* \brief Gestion des données envoyées par le processus 0 et reçues par tous les autres processus
* \author Richard Leestmans
* \version 0.1
* \date 11 Décembre 2022
*
* Partie du programme utilisée uniquement par les processus autres que le processus 0 et qui permet à ces derniers de décrypter les données envoyées par le processus 0,
* de les traiter, puis de renvoyer au processus 0 les résultats de ces traitements.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* \struct Data
* \brief Objet regroupant les données reçues par le processus 0.
*
* Objet regroupant toutes les informations envoyées par le processus 0 à tous les autres processus.
*/
typedef struct
{
    char*** grid; /*!< Grille contenant la portion du cube qui a été envoyée par le processus 0*/
    int numProc; /*!< Nombre de processus totaux utilisés dans l'exécution du programme*/
    int nbPlanes; /*!< Nombre de plans (feuillets) envoyés par le processus 0 à ce processus*/
    char rule[5]; /*!< règle utilisée pour l'analyse des cellules de la grille*/
    int gridSize; /*!< taille de la grille c'est-à-dire la longueur d'un côté du cube*/
}Data;

/**
* \fn Data* Data__create(char* data)
* \brief Constructeur de l'objet Data permettant de regrouper les informations envoyées par le processus 0.
* 
* \param data Chaine de caractères contenant l'ensemble des données envoyées par le processus 0
* \return Instance nouvelle allouée d'un objet de type Data
*/
Data* Data__create(char* data);
/**
* \fn int Data__getGridSize(Data* self)
* \brief Accesseur sur la taille de la grille.
* 
* \param self Instance d'un objet Data dans lequel récupérer la taille de la grille
* \return La taille de la grille, c'est-à-dire la longueur d'un des côtés du cube
*/
int Data__getGridSize(Data* self);
/**
* \fn void Data__storeData(Data* self, char* data)
* \brief Fonction permettant de stocker les différentes informations contenues dans les données, envoyées par le processus 0, dans les attributs de la structure. 
* 
* \param self Instance d'un objet Data dans lequel stocker les informations contenues dans les données envoyées par le processus 0
* \param data Données envoyées par le processus 0
*/
void Data__storeData(Data* self, char* data);
/**
* \fn void Data__dataToGrid(Data* self, char* data)
* \brief Fonction permettant de stocker des données contenues dans une chaine de caractères dans une grille à trois dimensions. 
* 
* \param self Instance d'un objet Data contenant la grille dans laquelle stocker les données
* \param data Données à stocker dans la grille
*/
void Data__dataToGrid(Data* self, char* data);
/**
* \fn char*** Data__cellNextGen(Data* self, int begin, int end)
* \brief Fonction permettant de déterminer le pattern de la nouvelle génération de cellules.
* 
* \param self Instance d'un objet Data contenant le pattern de la génération de cellules actuelle
* \param begin indice du premier feuillet sur lequel déterminer la génération de cellules suivantes
* \param end indice du dernier feuillet sur lequel déterminer la génération de cellules suivantes
* \return Une grille à 3 dimensions contenant la nouvelle génération de cellules
*/
char*** Data__cellNextGen(Data* self, int begin, int end);
/**
* \fn char* Data__gridToData(char*** grid, int gridSize, int nbPlanes, int begin, int end)
* \brief Fonction permettant de transformer la grille contenant la nouvelle génération de cellules en données sous la forme d'une chaine de caractères.
* 
* \param grid Grille contenant la nouvelle génération de cellules
* \param gridSize Taille de la grille (ici la longueur des côtés du cube selon les axes y et z)
* \param nbPlanes Nombre de plans de la grille (la longueur des côtés du cube selon l'axe x)
* \param begin Borne inférieure de l'intervalle des plans (feuillets) à stocker dans les données à retourner
* \param end Borne supérieure de l'intervalle des plans (feuillets) à stocker dans les données à retourner
* \return Les données résultantes contenues dans une chaine de caractères
*/
char* Data__gridToData(char*** grid, int gridSize, int nbPlanes, int begin, int end);
/**
* \fn char* Data__setNewGen(Data* self, int rank, int nbProc)
* \brief Fonction combinant la détermination de la nouvelle génération de cellules et le passage de cette nouvelle génération d'une grille à 3 dimensions vers une chaine de caractères.
* 
* \param self Instance d'un objet Data contenant la grille à partir de laquelle on détermine la nouvelle génération de cellules
* \param rank Rang du processus
* \param nbProc Nombre total de processus utilisés lors de l'exécution du programme
* \return Les données, contenues dans une chaine de caractères, à renvoyer au processus 0
*/
char* Data__setNewGen(Data* self, int rank, int nbProc);
/**
* \fn void Data__destroy(Data** self)
* \brief Destructeur d'une instance de Data.
* 
* \param self Adresse de l'instance à détruire
*/
void Data__destroy(Data** self);

#endif 