# MIF18 : Scheduling on Arduino -- Ordonnanceur

  * Grégoire Pichon, Université Lyon 1, LIP [email](mailto:gregoire.pichon@univ-lyon1.fr)
  * Version: 2024
  * Auteur original: Laure Gonnord
  * Inspiré d'un TP de Julien Forget et Thomas Vantroys, Lille
  * Autre collaborateur: Thierry Excoffier (Lyon 1).

## Rendu du TP

* Cf. [les instructions](http://laure.gonnord.pages.univ-lyon1.fr/advanced-systems/HOWTO_CR_TP_MIF18.md)
* Rendre uniquement un tgz après avoir fait un make clean (pas de binaire)
* Un répertoire à rendre par version de l'ordonnanceur
* Gardez en tête que l'objectif est d'analyser ce que vous faites et d'avoir du recul (par ex. par rapport à l'ordonnancement Linux)

## Description

* Dans ce TP, vous implémenterez un ordonnanceur simple pour Arduino en utilisant des timers et des interruptions

* Le [startup](_startup_part2.md) est à faire pour le Step 2, continuer avec le startup de la partie précédente pour le step 1

## Step 1 : deux leds en parallèle, avec des timers

Dans cet exercice, au lieu d'utiliser l'attente active (wait) en
bidouillant pour réaliser le parallélisme, on se propose d'utiliser
l'un des timers du micro-contrôleur pour faire clignoter les deux LEDs
à deux fréquences différentes, la LED rouge clignotera toutes les
200ms et la LED jaune toutes les 400 ms en parallèle. Le montage est
le suivant: led rouge sur analog 0, led jaune sur analog 1. On utilisera
DDRC et PORTC (de manière similaire à DDRB et PORTB dans la partie 1).

Afin de faire clignoter la LED jaune toutes les 400 ms en parallèle de
la LED rouge,  nous allons utiliser un timer:

* Le main continue d'allumer la LED rouge comme dans le TP précédent.

* Toutes les 400 ms, une fonction d'interruption sera appelée. Elle
  réalisera le changement d'état de la LED jaune, puis rendra la main. 

### Documentation

Explication des [timers/interruptions](ArduinoTimers.md)

### Expected work
Dans le fichier `main.c` du répertoire `timer_ex1`:

* Écrivez le code de la fonction `task_led_red` qui fait
  clignoter la LED rouge toutes les 200 ms (i.e., la LED est allumée
  pendant 200 ms, puis éteinte pendant 200 ms). Cette fonction est
  appelée dans la boucle de la fonction `main`. Testez le bon
  fonctionnement. 

* Remplissez la fonction `ISR` pour réaliser le changement d'état de
  la LED jaune.

Grâce à l'utilisation du timer, vous avez pu réaliser ainsi deux
tâches qui s'exécutent en parallèle.

## Step 2: Scheduler

Vous allez maintenant réaliser un véritable ordonnanceur utilisant
un algorithme _Round Robin_ avec un intervalle de temps de 20
ms. Votre micro-contrôleur réalisera trois tâches :

* faire clignoter la LED rouge toutes les 300 ms.

* envoyer en boucle un message sur le port série (chaque envoi de
  caractère sera espacé de 100 ms). Attention il faut envoyer un \n pour voir quelque chose !

* envoyer en boucle un message sur un mini écran LCD (avec une API fournie).

### Circuit

On a toujours les deux LEDs (rouge sur Analog 0, jaune sur Analog 1),
et on ajoute un écran LCD comme sur le dessin ci dessous:

<p align="center">
  <img width="1000" src="figs/leds_et_lcd_bb.png"/>
</p>

Les deux LEDs sont câblées comme précédemment.

* Le LCD est relié aux ports ``digital'' suivant lcd(4)-> D8, lcd 6->
  D9, lcd11 -> D4, lcd 12 -> D5, lcd13 -> D6, lcd14 -> D7.

* Le reste du branchement sert à assurer l'alimentation, la
  stabilité, et le contraste.
  
### Écriture série, stty et screen.

L'écriture port série s'effectue en fait sur
`/dev/ttyACM0`. Pour visualiser ce qui est écrit sur le port série,
nous utilisons `stty` pour configurer la lecture (voir le
Makefile), et le logiciel `screen`(voir
https://www.gnu.org/software/screen/), dans un terminal
différent de celui utilisé pour compiler/uploader:
```
screen /dev/ttyACM0 9600
```

Pour quitter proprement screen, on fera `CTRL-a`, puis `k`
(kill).

Alternativement vous pouvez directement faire :
```
make && make upload && (stty 9600 ; cat) < /dev/ttyACM0
```

### Travail préliminaire


Ici chaque tâche effectuera une boucle infinie (et les `delay_ms`
aussi). Le code fourni se trouve dans le répertoire `scheduler`:

* Testez la tâche LED rouge (de période 300 ms).

* À l'aide de la fonction `send_serial` fournie (qui envoie
  un caractère sur le port série), écrivez une tâche qui écrit le
  message de votre choix sur le port série en envoyant caractère par
  caractère toutes les 100 ms. Testez cette tâche.

* Écrivez le code de la tâche écrivant une chaîne sur l'écran LCD
  toutes les 400 ms. Vous utiliserez :

    * `lcd_write_string_4d(s)` pour imprimer une
    chaîne. Attention cette tâche elle même prend du temps (allez
    regarder le code de la librairie).

    * `lcd_write_instruction_4d(lcd_Clear)` pour effacer le
      texte courant.

* Testez individuellement chacune de ces tâches.


### Ordonnanceur RR sans sauvegarde de contexte

Maintenant que vos différentes tâches fonctionnent, vous pouvez
réaliser l'ordonnanceur. Vous devez commencer par créer une structure
(tableau de structs) représentant les différentes tâches (ici
3). Chaque structure de tâche sauvegardera ici son état (`RUNNING`,
`NOT_STARTED`):

```C
typedef struct task_s {
   volatile uint8_t state;         // RUNNING ou NOT_STARTED
   void (*start)(void);            //code for the task
} task_t;
```
L'ordonnanceur suivra donc l'algorithme suivant :
```C
  currentask <- nexttask()
  if currenttask.state == RUNNING //la tâche a déjà été interrompue
     then
          currenttask.restart();  // en vrai, utilise la fonction start en pensant à reactiver les interruptions avec sei().
     else                         // premier lancement de la tâche
          currenttask.state = RUNNING ;
          sei();                  //permettre les interruptions. 
          currenttask.start();
  endif
```

Codez cet ordonnanceur (répertoire scheduler/) :

* Vous utiliserez toujours un timer qui lancera une interruption régulièrement.

* La période de l'ordonnanceur sera de 20 ms. Pour faciliter le debug, chaque fois
  que l'ordonnanceur s'exécute, vous allumerez la LED jaune.

* Evidemment, comme il y a une interruption toutes les 20ms, les périodes des tâches
  n'ont plus vraiment de sens, on ne demande pas ici de modifier ce comportement.

* Mettez en évidence le problème de non-restoration de contexte :
  les tâches reprennent toujours au début.

* Après avoir testé, sauvez cette v1 dans un répertoire à part `scheduler/V1/` (dans V1,
  il y aura un code complet qui compile, commenté, avec un readme, et quelques lignes
  d'explication des fonctionnalités et ce que vous observez).

### Ordonnanceur avec sauvegarde de contexte

Pour pouvoir reprendre l'exécution d'une tâche au bon endroit
après interruption, il faut sauvegarder le contexte, ie l'ensemble
des valeurs des registres à une certaine adresse dans la pile (SP) au moment de
son interruption. On modifiera la structure de tâche en:

```C
typedef struct task_s {
        volatile uint16_t stack_pointer; // variable pour stocker SP.
        volatile uint8_t state;          // toujours RUNNING ou NOT_STARTED
        // here you can add the associated task
} task_t;
```

Pour une explication du volatile [clic](https://en.wikipedia.org/wiki/Volatile_(computer_programming))

#### Un point sur le changement de contexte

Comme on l'a déjà vu en architecture, pour réaliser le changement de contexte, il est nécessaire de
sauvegarder la valeur des différents registres du micro-contrôleur (Lire aussi
[multitasking](https://xivilization.net/~marek/binaries/multitasking.pdf))

Pour cela, on vous fournit deux macros `SAVE_CONTEXT` et
`RESTORE_CONTEXT`. Pour la gestion de la mémoire des
différents processus, nous allons découper l'espace mémoire. Dans un
AVR atmega328p, la mémoire est utilisée en commençant par les adresses
les plus élevées. La tâche "écriture série" débutera son utilisation
mémoire à l'adresse 0x0700, la tâche "écriture écran LCD" débutera à
l'adresse 0x0600 et la tâche "LED" débutera à l'adresse 0x0500. Ces
adresses représentent le départ du pointeur de pile (Stack
Pointer). Ce dernier est rendu directement accessible par
`gcc-avr` via la "variable" SP.

L'ordonnanceur réalisera donc l'algorithme suivant :

```C
  SAVE_CONTEXT();       // sur la pile
  currenttask.saveSP(); // sauver le SP dans la tâche
  sei();                // permettre les interruptions
  currenttask <- nexttask()
  SP = currenttask.getSP();
  si currenttask.state == RUNNING //la tâche a déjà été interrompue
     alors 
          RESTORE_CONTEXT()
     sinon             // premier lancement de la tâche
          currenttask.state = RUNNING ;
          currenttask.start();
  finsi
```


####  TODO : Dans le main:

* Observez les deux macros de stockage du contexte. Attention,
  dans ces macros, on appelle la fonction `cli` pour stopper les
  interruptions. La conséquence est qu'il faudra relancer `sei` à
  chaque reprise en main de l'ordonnanceur.

* Modifier l'ordonnanceur Round Robin précédent (avec un
  intervalle de 20 ms)  et testez-le.

* Sauvegardez cette version dans `scheduler/V2/`


### Ordonnanceur avec ressource partagée

On souhaite maintenant rajouter une tâche supplémentaire qui envoie en
boucle le caractère `'@'`  sur le port série. Ceci nécessite donc
de gérer l'accès concurrent au port série entre deux tâches.
Pour un cas plus réaliste, vous pouvez imaginer deux tâches qui veulent faire
des logs via le port série.

Vous allez mettre en place un mécanisme simplifié de sémaphores
permettant d'assurer l'accès en exclusion mutuelle au port série. 

Voici une solution possible: chaque tâche a désormais trois statuts
possibles: `CREATED` (la
tâche n'a encore jamais été exécutée), `ACTIVE` (la tâche est
disponible) et `SUSPENDED` (la tâche attend l'accès à une ressource
partagée).


#### TODO

* Faites une étude sérieuse du problème. Il est indispensable que
  cette étude soit faite, avec les primitives suivantes en pseudo code
  plutôt qu'une implementation buguée. Des dessins scannés peuvent
  aider pour les explications

* Écrivez une primitive `take_serial` (similaire au P/Wait
  des sémaphores), qui vérifie si le port série est disponible, si oui
  le prend, si non la tâche l'exécutant est suspendue.

* Écrivez une primitive `release_serial` (similaire au
  V/Signal des sémaphores), qui rend le port série et, si besoin, rend
  active la tâche suspendue.

* Modifiez le code des tâches et de l'ordonnanceur en
  conséquence. Notez bien que les primitives `take_serial` et
  `release_serial` doivent être ininterruptibles !

* Commentez correctement votre code, ajoutez dans le rapport le statut
  de votre implementation. Sauvegardez cette version dans
  `scheduler/V3/`

* Est-ce que l'implémentation de votre ordonnanceur est efficace ?
  Quelle est la complexité dans le pire des cas ?

* Comparez votre approche avec les observations que vous avez pu faire lors du TP "Ordonnancement Linux"

* Ajouter des priorités aux tâches. Comment éviter la famine ?

## Tinkercad pour celles et ceux qui souhaitent terminer sur un simulateur en ligne

* Vous devez commencer par créer un compte.

* Ensuite aller dans la partie "Circuits" puis "Créer un circuit".

* Vous pouvez ensuite réaliser le circuit avec un Arduino, un board, des LEDs, un LCD...

* Pour les résistances, attention de bien définir la bonne capacité.

* Ensuite allez dans la partie "code", sélectionnez "Texte" puis vous pouvez coder.

* Pour lancer le code sur l'Arduino, utilisez "Démarrer la simulation".

* Il n'y a pas de `main()` dans le code, vous devez compléter la fonction `setup()` pour l'initialisation et la fonction `loop()` qui tourne dans une boucle infinie pour le reste.

* Adaptez le code fourni pour [TinkerCad](http://tinkercad.com): il faut déplacer et mettre dans `setup()` et `loop()`.

* Pour la partie ordonnancement, un squelette vous est fournit pour Tinkercad. Il contient directement toutes les librairies pour éviter les problèmes de compilation.
