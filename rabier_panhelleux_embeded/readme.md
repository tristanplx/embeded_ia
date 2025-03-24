
# Configuration du Linker dans STM32CubeIDE

Comme précisé dans le readme principal, on rencontre énormément d'erreur dans la configuration du linker. Il est donc nécessaire de refaire la configuration de ce dernier régulièrement.
Les étapes suivantes permettent de modifier le linker et de configurer le chemin de la librairie dans STM32CubeIDE.

1. **Ouvrir les propriétés du projet**  
   Aller dans le menu contextuel (clic droit) sur le projet et sélectionner **Properties**.

2. **Accéder aux paramètres du build**  
   Aller dans **C/C++ Build** > **Settings**, puis se rendre dans l'onglet **Tool Settings**.

3. **Modifier la configuration du linker**  
   Aller dans **MCU GCC Linker > Libraries**.
   - Dans **Library search path (-L)**, ajouter le chemin vers le dossier contenant la librairie. Par exemple :  
     ```
     ../Middlewares/ST/AI/Lib
     ```
   - Dans **Libraries (-l)**, ajouter uniquement le nom de la librairie sans le préfixe `lib` et sans l'extension `.a`.  
     Pour une librairie nommée `libNetworkRuntime1000_CM4_GCC.a`, inscrire :  
     ```
     NetworkRuntime1000_CM4_GCC
     ```

4. **Appliquer les modifications et reconstruire le projet**  
   Faire **Apply and Close** pour enregistrer les modifications, puis aller dans **Project > Clean Project** suivi de **Build Project** afin de reconstruire le projet.

Normalement il est alors possible de faire le '**Validate on target**' ainsi que **run** le projet.
