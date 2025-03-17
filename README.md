# embedded_ia

## Contexte et Objectif

Le but de cette session pratique est de développer un modèle de deep learning capable de réaliser une mission de maintenance prédictive sur des machines industrielles. Pour ce faire, nous utilisons le dataset **AI4I 2020 Predictive Maintenance**, qui regroupe 10 000 relevés de capteurs et des étiquettes indiquant si une machine a subi une défaillance et, le cas échéant, le type de défaillance (TWF, HDF, PWF, OSF, RNF).

## Démarche Globale

### Jupyter Notebook

1.  **Analyse Exploratoire des Données :**  
    Nous avons commencé par examiner la distribution des machines fonctionnelles versus défaillantes ainsi que la répartition des différents types de défaillance. Cette analyse a mis en évidence un déséquilibre important, avec une surreprésentation des machines fonctionnelles, ce qui risquait de biaiser l’apprentissage du modèle.
    
2.  **Préparation des Données :**  
    Après avoir nettoyé et sélectionné les variables pertinentes (ex. : températures, vitesse, couple, usure, etc.), nous avons divisé le dataset en ensembles d’entraînement, de test et de validation. Pour contrer le déséquilibre, nous avons appliqué la technique SMOTE et utilisé la pondération des classes lors de l’entraînement.
    
3.  **Modélisation :**  
    Notre modèle est un réseau de neurones profond constitué de plusieurs couches denses, intégrant la normalisation (BatchNormalization), l’activation ReLU, un mécanisme de régularisation par L2 et du dropout pour limiter le surapprentissage. La couche de sortie utilise une activation softmax pour prédire les 5 classes (les 4 types de défaillance et l’état fonctionnel).
    
4.  **Évaluation :**  
    Nous avons évalué la performance du modèle à l’aide d’indicateurs tels que la matrice de confusion et le rapport de classification. L’approche initiale sans rééquilibrage montre une forte prédominance de la classe fonctionnelle, tandis que le modèle entraîné sur des données rééquilibrées améliore significativement la détection des classes de défaillance minoritaires.

### STM32Cube   

   ## Arborescence
   Le dossier **numpy_data** contient tous les dataset d'entrainement, de test et de validation. Cela permet notamment de charger ces données dans STM32mxAI afin de réaliser l'analyse du modèle.
   Le dossier **rabier_panhelleux_embedded** contient tous les fichiers et dossiers nécessaires à l'implémentation du modèle sur la carte *STM32L4R9I-DISCO*, le modèle étant chargé sous forme d'un fichier '.h5' dans le repertoire racine.
   

## Conclusion
