# MIF18-Lab TP2: Mémoire virtuelle

  * Grégoire Pichon, Université Lyon 1, LIP [email](mailto:gregoire.pichon@univ-lyon1.fr)
  * Version: 2022

## Description du TP

L'objectif de ce TP est d'observer le comportement mémoire de
différents programmes. Nous commencerons par observer les défauts de
page apparaissant lors de la lecture / écriture d'un gros fichier. Par
la suite, nous observerons le comportement de l'OS et l'organisation
de l'espace virtuel des processus. Nous verrons notamment comment le
système fait de l'allocation paresseuse et la gestion de la pile et du
tas. Pour finir, des mappings permettront de mettre en place des zones
de partage entre processus.

## Step 1: Organisation de l'espace virtuel des processus

### Step 1a: Quelques observations

* Déduire de ``/proc/self/maps`` comment les différentes régions
mémoire des processus sont organisées en mémoire virtuelle sous Linux.
Pourquoi la pile et le tas sont-ils placés ainsi ?

* Où est située la mémoire du noyau ? Pourquoi fait-elle partie de
  l'espace virtuel utilisateur du processus alors qu'il n'y a pas
  accès ?

* Quelles variations observe-t-on d'un processus à l'autre, même s'ils
sont lancés exactement de la même façon ?  Pourquoi est-il utile de
faire varier le mapping en espace utilisateur ?  Pourquoi est-il utile
de ne pas faire varier le mapping noyau ?

Afin de relancer un même programme plusieurs fois (cat par exemple),
on pourra utiliser ``watch -n 1``, qui le relancera toutes les
secondes.

Pour la suite de cette partie, on pourra utiliser les lignes suivantes afin d'afficher les VMA du processus courant:

```C
  char s[64];
  sprintf(s, "cat /proc/%u/maps", getpid());
  system(s);
```

### Step 1b: Mémoire du tas

* Écrire un programme qui fait plein de petits ``malloc`` (quelques
kilooctets ou dizaines de kilooctets) et expliquer l'évolution du (ou
des) tas.  Faire maintenant des gros ``malloc`` (de l'ordre du
mégaoctet) et expliquer également. En déduire le comportement du
système vis-à-vis d'un éventuel débordement du tas.

### Step 1c: Mémoire de la pile

* Écrire un programme récursif qui va faire déborder la pile. Pour
savoir à quel moment il a planté, on pourra lui faire afficher
``/proc/self/maps`` régulièrement.

* On pourra également essayer de rattraper le signal SIGSEGV pour
afficher l'adresse exacte où le débordement a eu lieu (cela
nécessitera probablement ``sigaltstack``). Cette partie étant plus
complexe à mettre en place, revenez dessus à la fin du TP si vous avez
le temps uniquement.

Pourquoi le système ne peut-il pas gérer les débordements de pile comme ceux du tas ?

### Step 1d: Mapping en mémoire

* Faire quelques ``mmap`` privés et partagés et retrouver les entrées correspondantes
dans ``/proc/self/maps``.

* Déterminez la taille maximale que l'on peut projeter en mémoire virtuelle d'un
seul coup avec ``mmap``. On utilisera les flags ``MAP_ANONYMOUS|MAP_PRIVATE`` et
le descripteur ``-1`` pour obtenir une projection anonyme (sans fichier associé).
Essayez en ajoutant en plus le flag ``MAP_NORESERVE`` et expliquez.

* Faites une boucle de gros ``mmap`` jusqu'à ce que le système refuse
de projeter de nouvelles zones. Déduisez-en une approximation de la
taille maximale de l'espace d'adressage du processus.  Comparez cette
taille à la notion d'architecture 64bits, notamment en regardant le
fichier ``/proc/cpuinfo``.

## Step 2: Manipulation de la mémoire virtuelle

### Step 2a: Mappings partagés d'un même fichier

* Créez un fichier de 8ko. Vous pouvez utiliser la commande suivante (avec les bons paramètres)
``
 dd if=/dev/zero of=fichier count=100000 bs=1000
``


* Écrivez un programme qui projette (``mmap``) trois fois ce fichier dans un même
processus de manière publique (``MAP_SHARED``).

* Écrivez 1111 au début d'un mapping,
puis 2222 dans second mapping (décalé de 4 octets),

* Observez le résultat depuis le troisième mapping, en affichant les 8 premiers octets.

### Step 2b: Copy-On-Write
On utilise maintenant un mapping public et un mapping privé vers le même fichier.

* Écrivez 1111 au début du premier mapping puis 2222 dans le second mapping (avec un décalage de 4 donc)

* Écrivez le contraire dans la deuxième page (2222 décalé de 4096+4 dans le second mapping,
puis 1111 décalé de 4096 dans le premier).

En réfléchissant à l'implémentation du copy-on-write, expliquez le phénomène.
Que se passerait-il si on écrivait à cheval sur les deux pages ?
Comment pourrait-on utiliser ce phénomène pour mesurer la taille des pages ?

### Step 2c: Mesures des défauts de page
Modifiez votre programme pour ne créer qu'un seul mapping public de 1Mo
(et aucun privé).
Ajoutez un argument en ligne de commande pour terminer après une des
étapes intermédiaires:

* avant le mmap,
* après le mmap,
* après une lecture d'un octet, d'une page, ou de tout le mapping,
* après une écriture d'un octet, d'une page, ou de tout le mapping,
* jusqu'à la fin du programme.

Utilisez ``/usr/bin/time`` pour mesurer les défauts de pages de
votre processus selon l'étape à laquelle vous vous arrêtez.
Expliquez les chiffres.

