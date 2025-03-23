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
    Le modèle commence par une couche de 512 neurones, puis enchaîne avec des couches plus petites (128, 64 et 32 neurones), chacune suivie d’un Dropout de 10 % pour éviter le surapprentissage. Toutes ces couches utilisent l’activation ReLU.
La dernière couche utilise softmax, ce qui permet de prédire l’une des 5 classes possibles (les 4 types de panne ou un fonctionnement normal). On utilise 70 epochs et 100 de batch.
    
4.  **Évaluation :**  
    Nous avons évalué la performance du modèle à l’aide d’indicateurs tels que la matrice de confusion et le rapport de classification. L’approche initiale sans rééquilibrage montre une forte prédominance de la classe fonctionnelle, tandis que le modèle entraîné sur des données rééquilibrées améliore significativement la détection des classes de défaillance minoritaires.

### STM32Cube
    Dans STM32CubeIDE

   ## Arborescence
   Le dossier **rabier_panhelleux_embedded** contient tous les fichiers et dossiers nécessaires à l'implémentation du modèle sur la carte *STM32L4R9I-DISCO*, le modèle étant chargé sous forme d'un fichier '.tflite' dans le repertoire racine. Les jeux de connées pour la validation sont également présent dans le dossier utilisé pour le code STM32.
   Le repertoire racine contient le modèle et le fichier jupyter Notebook. On a également un fihcier python directement pour compiler directement le modèle et pouvoir l'exporter.
   

## Conclusion
