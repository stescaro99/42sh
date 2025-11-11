# 42sh — Subject (Italiano)

> 42sh: il ritorno della vendetta dal passato!

## Capitolo IV — Istruzioni generali

- Il file eseguibile deve chiamarsi `42sh`.
- Il `Makefile` deve compilare il progetto e contenere le regole usuali. Deve ricompilare e rilinkare il programma solo se necessario.
- Se siete furbi, userete la vostra libreria per il vostro `42sh`. Inviate anche la cartella `libft` inclusa con il proprio `Makefile` alla radice del repository. Il vostro `Makefile` dovrà prima compilare la libreria e poi il progetto.
- Dovete gestire gli errori con cura. In nessun caso il vostro programma può terminare in modo inaspettato (segmentation fault, bus error, double free, ecc.).
- Il terminale non deve mostrare caratteri illeggibili: siate attenti alle impostazioni.
- Il programma non deve avere perdite di memoria (memory leaks).

> 42sh — Il ritorno della vendetta dal passato!

- Nella parte obbligatoria, potete usare solo le seguenti librerie/funzioni:
  - L’intera libc.
  - La libreria `readline` (`-lreadline`).
  - Le librerie `termcap` e `ncurses` (`-ltermcap` e `-lncurses`).
- Per la parte bonus, potete usare altre funzioni o librerie purché l’uso sia giustificato durante la difesa. Siate intelligenti nelle scelte!

## Capitolo V — Parte obbligatoria

### Prerequisiti di Minishell

- Visualizzazione del prompt.
- Esecuzione dei comandi con i loro parametri e gestione del `PATH`.
- Gestione corretta di spazi e tabulazioni.

### Prerequisiti di 42sh

- Editing completo della riga di comando.
- Operatori di redirezione e aggregazione:
  - `>`
  - `>>`
  - `<`
  - `<<`
  - `>&`
  - `<&`
- Pipe: `|`
- Separatore: `;`
- Built-in richiesti:
  - `cd`
  - `echo`
  - `exit`
  - `type`
- Operatori logici: `&&` e `||`.

### Precedenza degli operatori

Gli operatori di controllo `;`, `&&`, `||` e gli operatori di redirezione `<`, `>`, `|` hanno una precedenza. Fate attenzione!

Per esempio, questi due comandi non producono lo stesso risultato:

```sh
ls doesnotexist .  2>&1 >/dev/null
```

```sh
ls doesnotexist .  >/dev/null 2>&1
```

Non dimenticate di verificare la grammatica della vostra shell.

### Variabili interne della shell

- Gestione delle variabili interne della shell (ignorate le variabili di sola lettura).
  - Creazione di variabili interne con la sintassi: `name=value`.
  - Esportazione delle variabili interne nell’ambiente tramite il built-in `export`.
  - Possibilità di elencare le variabili interne della shell tramite il built-in `set` (nessuna opzione richiesta).
  - Rimozione di variabili interne e d’ambiente tramite il built-in `unset` (nessuna opzione richiesta).
  - Creazione di una variabile d’ambiente per un singolo comando, ad esempio: `HOME=/tmp cd`.
  - Espansione semplice dei parametri con la sintassi `${}` (nessun formato aggiuntivo richiesto).
  - Accesso al codice di uscita dell’ultimo comando tramite l’espansione `${?}`.
- Gestione del job control con i built-in `jobs`, `fg`, `bg` e l’operatore `&`.
- Gestione corretta di tutti i segnali.
- Ogni built-in deve offrire le opzioni previste dallo standard POSIX, tranne nei casi espliciti come `set` o `unset`.

#### Spiegazione di `env`, `setenv`, `unsetenv`

- `env` è un binario e non un built-in nelle shell. Vi è stato chiesto in passato di implementarlo per comprenderne il comportamento; questa volta NON è richiesto fornirlo.
- I built-in `setenv` e `unsetenv` sono esclusivi della famiglia di shell CSH. Non copiate il comportamento di queste shell. Vedi anche: “Csh Considered Harmful”.

## Capitolo VI — Parte modulare

Le funzionalità richieste nella parte obbligatoria rappresentano il minimo indispensabile. Dovete ora selezionare e implementare funzionalità più avanzate. È richiesto un minimo di 6 funzionalità dalla lista seguente per convalidare il progetto.

Ricordate però che la stabilità è molto più importante della quantità. Non includete opzioni che causano problemi al resto del programma. Questa parte verrà valutata solo se la parte obbligatoria è completa e indistruttibile.

Funzionalità modulari:

- Inibitori: `"` (doppio apice), `'` (apice singolo) e `\` (backslash).
- Pattern matching (globbing): `*`, `?`, `[]`, `!` e intervalli di caratteri con `\` (escape).
- Tilde expansion e formati aggiuntivi per i parametri:

  - `~`
  - `${parameter:-word}`
  - `${parameter:=word}`
  - `${parameter:?word}`
  - `${parameter:+word}`
  - `${#parameter}`
  - `${parameter%}`
  - `${parameter%%}`
  - `${parameter#}`
  - `${parameter##}`
- Gruppi di controllo e subshell: `()`, `{}`.
- Sostituzione di comandi: `$()`.
- Espansione aritmetica: `$(( ))` con soli i seguenti operatori:

  - Incremento/decremento: `++`, `--`
  - Addizione, sottrazione: `+`, `-`
  - Moltiplicazione, divisione, modulo: `*`, `/`, `%`
  - Confronti: `<=`, `>=`, `<`, `>`
  - Uguaglianza, disuguaglianza: `==`, `!=`
  - Logici AND/OR: `&&`, `||`

  Consultate la documentazione su precisione e operazioni aritmetiche per maggiori dettagli sul comportamento degli operatori.
- Process substitution: `<()`, `>()`.
- Gestione completa della cronologia:

  - Espansioni:
    - `!!`
    - `!word`
    - `!number`
    - `!-number`
  - Salvataggio su file per utilizzo tra sessioni.
  - Built-in `fc` (tutte le opzioni POSIX).
  - Ricerca incrementale nella cronologia con `CTRL-R`.
- Completamento dinamico contestuale di comandi, built-in, file, variabili interne e d’ambiente.

  - Esempio: digitando `ls /` e con il cursore su `/`, il completamento deve proporre solo il contenuto della root e non comandi o built-in.
  - Esempio: con `echo ${S`, il completamento deve proporre solo i nomi di variabili che iniziano per `S`, interne o d’ambiente.
- Due modalità di editing della riga di comando: stile Vi e Readline. La scelta tra le modalità avviene tramite l’opzione `-o` del built-in `set`.

  - Scorciatoie Vi: `#`, `v`, `j`, `k`, `l`, `h`, `w`, `W`, `e`, `E`, `b`, `B`, `^`, `$`, `0`, `|`, `f`, `F`, `;`, `,`, `a`, `A`, `i`, `I`, `r`, `R`, `c`, `C`, `S`, `x`, `X`, `d`, `D`, `y`, `Y`, `p`, `P`, `u`, `U`.
  - Scorciatoie Readline: `C-b`, `C-f`, `C-p`, `C-n`, `C-_`, `C-t`, `A-t`.

  Le spiegazioni delle scorciatoie Vi si trovano nell’implementazione di `sh`, in particolare nella sezione “EXTENDED DESCRIPTION”. Per Readline, consultate la documentazione ufficiale.
- Gestione degli alias tramite i built-in `alias` e `unalias`.
- Tabella hash e built-in `hash` per interagirvi.
- Built-in `test` con i seguenti operatori: `-b`, `-c`, `-d`, `-e`, `-f`, `-g`, `-L`, `-p`, `-r`, `-S`, `-s`, `-u`, `-w`, `-x`, `-z`, `=`, `!=`, `-eq`, `-ne`, `-ge`, `-lt`, `-le`, `!`, nonché la possibilità di un operando semplice senza operatore.

> In caso di dubbio sul comportamento di una funzionalità, fate riferimento allo standard POSIX. È perfettamente accettabile implementare una funzionalità in modo diverso, se coerente per la vostra shell; non è invece accettabile fare meno di quanto previsto dalle specifiche POSIX per "pigrizia".

## Capitolo VII — Parte bonus

### VII.1 Elenco dei possibili bonus

- Shell scripting (`while`, `for`, `if`, `case`, `function`, ecc.).
- Autocompletamento per i parametri di comandi/built-in.
- Una shell conforme allo standard POSIX.

### VII.2 Prerequisiti per considerare i bonus

Nuova regola sui bonus di 42sh: verranno considerati solo se il vostro codice è chiaro e pulito.

Cosa intendiamo?

- Niente operatori ternari ogni 3 righe.
- Nomi di funzione espliciti (niente `ft_parse1`, `ft_parse2`, ecc.).
- Lo stesso vale per i nomi delle variabili.
- Usate con giudizio il qualificatore `const`.
- Mantenete uno storico git e messaggi di commit espliciti.
- Abbiate test automatizzati.

La parte bonus sarà valutata solo se la parte obbligatoria è PERFETTA. “Perfetta” significa che la parte obbligatoria è stata realizzata integralmente e funziona senza malfunzionamenti. Se non avete soddisfatto TUTTI i requisiti obbligatori, la parte bonus non verrà valutata.
