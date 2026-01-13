# 42sh — Guida completa all’implementazione

Questo documento ti guida passo–passo a costruire una shell UNIX stabile e completa (42sh), facendo leva su quanto già fatto in Minishell. L’obiettivo è consegnare un’implementazione robusta, senza crash, senza memory leaks, con precedenze corrette e un’esperienza d’uso intuitiva.

---

**Obiettivi**
- **Stabilità**: nessun segfault/double free; error handling chiaro; memoria gestita correttamente.
- **Funzionalità base**: prompt, esecuzione comandi con `PATH`, redirezioni, pipe, separatori `;`, logici `&&`/`||`, subshell `(...)`.
- **Linea di comando**: editing (Readline), cronologia; modalità Vi via `set -o`.
- **Variabili**: interne + ambiente; espansioni `${}`, `$?`.
- **Job control**: `&`, `jobs`, `fg`, `bg`.

---

**Dipendenze e build**
- **Librerie**: `libc`, `readline` (`-lreadline`), termcap/ncurses (`-ltermcap`, `-lncurses`) dove richiesto.
- **Libft**: includi la tua `libft/` con relativo `Makefile` e falla compilare prima del progetto.
- **Compilatore/flag**: `cc` con `-Wall -Wextra -Werror`.
- **Consiglio (Windows)**: usa WSL/WSL2 o una VM Linux per compilare e testare `readline`/`termcap`.

Esempio di build (quando hai il tuo `Makefile` pronto):
```sh
make -C libft re
make
./42sh
```

---

**Struttura consigliata (modulare)**
- `src/lexer/` — tokenizzazione: parole, quote, operatori (`|`, `;`, `&&`, `||`, `<`, `>`, `>>`, `<<`, `>&`, `<&`), FD.
- `src/parser/` — costruzione AST o piano di esecuzione con precedenze.
- `src/expansion/` — rimozione quote, `$VAR`, `${}`, `$?`, tilde, globbing (se abilitato).
- `src/executor/` — esecuzione: pipeline, redirezioni/aggregazioni, subshell, logici, `;`.
- `src/builtins/` — `cd`, `echo`, `exit`, `type`, `export`, `unset`, `set`, `pwd`, `env`.
- `src/env/` — variabili ambiente + interne; promozione con `export`.
- `src/job/` — job control (`&`, `jobs`, `fg`, `bg`).
- `src/signal/` — gestione segnali (compatibile con Readline).
- `src/ui/` — prompt, integrazione Readline, history.
- `src/utils/` — utility (stringhe, array, error, fd, path, gc).

Nota: puoi riusare parti di Minishell dove già implementate (pipe/redir/handle_quote/exec/segnali) adattando interfacce.

---

**Strutture dati consigliate (e come usarle)**
- Basate su quanto già presente in [42sh/src/42sh.h](42sh/src/42sh.h):

  - `t_env`
    - Scopo: lista collegata di variabili d’ambiente esportabili.
    - Campi utili: `name`, `value`, `unsetted`, `next`.
    - Uso: costruisci/aggiorna la lista; genera `char **envp` solo al momento di `execve` (meglio evitare di tenerlo per–nodo; se già presente, considera di spostarlo in `t_data` o rigenerarlo on–demand).
    - API: `export` promuove da `t_memory`; `unset` marca `unsetted` o rimuove; helper per serializzare `envp`.

  - `t_memory`
    - Scopo: variabili interne della shell (non esportate).
    - Campi: `name`, `value`, `next`.
    - Uso: gestisci assegnazioni `name=value` in input; expansions leggono prima da `t_memory`, poi da `t_env`.

  - `t_alias`
    - Scopo: mappa alias `name -> value`.
    - Campi: `name`, `value`, `next`.
    - Uso: espansione pre–parse del primo token comando; built–in `alias`/`unalias` manipolano la lista.

  - `t_line`
    - Scopo: rappresenta un’unità logica della command line/AST semplificato.
    - Campi: `type` (tipo nodo), `line` (testo normalizzato), `command_path` (risolto da PATH), `logic` (relazione con `next`: NONE/SEQ/AND/OR), `exit` (ultimo status), `parenthesis` (lista figlia per `(...)`), `next`.
    - Uso: crea una lista di `t_line` per i livelli `;`, `&&`, `||`; nei nodi memorizza pipeline/redi locali (vedi sotto strutture complementari).

  - `t_data`
    - Scopo: contesto runtime della shell.
    - Campi: `env`, `memory`, `alias`, `line` (radice/attuale), `fds` (array per fd globali).
    - Estensioni consigliate: `last_status` (int), `hash` (cache PATH), `jobs` (tabella job), `history` handler, flag/bitmask per stato.

- Strutture complementari (minime, modulari):
  - `t_redir`: `{ int src_fd; int op; char *target; bool append; bool heredoc; bool aggregate; }`
    - Gestisce `<`, `>`, `>>`, `<<`, `<&`, `>&`. Per heredoc, aggiungi `char *delim; bool delim_quoted;` e staging file.
  - `t_cmd`: `{ char **argv; t_redir *redir_list; size_t redir_len; }`
    - Un comando con i suoi argomenti e redirezioni locali.
  - `t_pipeline`: `{ t_cmd *cmds; size_t count; }`
    - Un gruppo `cmd | cmd | ...`; applicato dentro il nodo `t_line` corrispondente.
  - `t_job`: `{ int pgid; int state; char *cmdline; }` (opzionale per job control).

- Note di progetto
  - Mantieni ownership chiara: l’esecutore libera `argv`/redir post–esecuzione; il parser libera AST su fine ciclo.
  - Evita duplicazioni di `envp` e preferisci rigenerare l’array quando richiesto da `execve`.
  - Per compatibilità Norm, se eviti `enum`, usa `short`/`int` con costanti per `type`/`logic`.

---

**Ordine di lavoro (milestone + come farle)**

1) **Bootstrap & loop interattivo**
- **Cosa**: setup di `main` con Readline, prompt e gestione EOF (`Ctrl-D`).
- **Come**: integra `readline`, `add_history`, valida input vuoto/solo spazi; una sola variabile globale (`volatile sig_atomic_t`) per numero di segnale.
- **Test**:
  ```sh
  echo hello
  Ctrl-C  # deve mostrare un nuovo prompt
  Ctrl-D  # deve uscire se la riga è vuota
  ```

2) **Lexer robusto (quote/operatori/FD)**
- **Cosa**: tokenizza stringa in `TOKEN{type, value, pos}`; gestisci `'"` e `\`, operatori (`| ; && || < > >> << >& <&`), numeri di FD (`2>`, `2>&1`), parentesi `()`.
- **Come**: automa carattere–per–carattere; regole quote: in `'...'` nessuna espansione; in `"..."` consenti `$VAR`, disabilita globbing/split.
- **Errori**: quote non chiuse → status 2 con messaggio.
- **Test**:
  ```sh
  echo "a b" 'c d' e\ f | wc -w
  ls x 2>&1 >/dev/null
  (echo a ; echo b) | wc -l
  ```

3) **Parser con precedenze**
- **Cosa**: costruisci AST/plan con livelli: `;` > `&&/||` > `|` > redirezioni.
- **Come**: grammar tipo EBNF:
  - `line := and_or (';' and_or)*`
  - `and_or := pipeline (('&&'|'||') pipeline)*`
  - `pipeline := command ('|' command)*`
  - `command := [assign|redir]* simple_cmd [redir]* | subshell`
  - `redir := [FD] ('>'|'>>'|'<'|'<<'|'<&'|'>&') WORD`
  - `subshell := '(' line ')'
- **Rappresentazione**: usa nodi per `;`, `&&`, `||`; pipeline come lista di comandi con redir locali; `(...)` come nodo con figlio.
- **Test**:
  ```sh
  false && echo NO || echo YES ; echo DONE
  ls x >/dev/null 2>&1  # differente da 2>&1 >/dev/null
  ```

4) **Heredoc e regole di espansione**
- **Cosa**: `<<` con delimitatore; se quotato, nessuna espansione nel corpo.
- **Come**: rileva quote sul delimiter in lexing; marca il contesto heredoc; leggi fino alla linea esatta del delimiter.
- **Test**:
  ```sh
  cat <<EOF
  hello $USER
  EOF
  cat <<'EOF'
  hello $USER
  EOF
  ```

5) **Espansioni minime**
- **Cosa**: `$VAR`, `${VAR}`, `${?}`; tilde; quote removal.
- **Come**: applica nell’ordine: param espansion → tilde → field splitting/globbing (se abilitato) → quote removal.
- **Test**:
  ```sh
  echo $HOME ${?}
  HOME=/tmp echo $HOME
  echo "literal $HOME"
  ```

6) **Executor (comando semplice, PATH, redirezioni, pipe)**
- **Cosa**: esegui un comando con `execve`; gestisci `PATH`; applica redirezioni prima di `execve`; pipeline con `dup2`.
- **Come**: per pipeline crea N-1 pipe; forka per ogni segmento; duplica STDIN/STDOUT; chiudi fd non usati.
- **Test**:
  ```sh
  /bin/echo ok
  echo ok | wc -c
  ls >out.txt ; wc -l < out.txt
  ```

7) **Logici `&&`/`||` e separatore `;`**
- **Cosa**: valuta condizionali usando l’exit status del nodo precedente.
- **Come**: `AND`: esegui il successivo solo se exit==0; `OR`: esegui solo se exit!=0; `;`: sempre.
- **Test**:
  ```sh
  false && echo A || echo B ; echo C
  ```

8) **Subshell `(...)`**
- **Cosa**: esegui un gruppo in un processo figlio con proprio contesto.
- **Come**: forka; ricostruisci (o condividi con regole chiare) env/memoria; esegui lista figlia; riporta exit status.
- **Test**:
  ```sh
  (echo a ; echo b) | wc -l
  (cd / ; pwd) ; pwd
  ```

9) **Built-in principali**
- **Cosa**: `cd`, `echo`, `exit`, `type`, e quelli ereditati da Minishell (`pwd`, `env`, `export`, `unset`, `set`).
- **Come**: built-in che cambiano stato shell (es. `cd`, `export`, `unset`, `exit`) vanno eseguiti nel processo principale; dentro pipeline puoi eseguire in child (attenzione agli effetti).
- **Test**:
  ```sh
  echo -n "hi" ; echo type echo
  cd / ; pwd
  export A=1 ; env | grep '^A='
  unset A ; set | grep '^A=' || echo missing
  ```

10) **Job control**
- **Cosa**: `&` (background), `jobs`, `fg`, `bg`.
- **Come**: mantieni tabella job con pgid/statI; gestisci segnali e terminal control; `fg` porta il job in foreground e attende.
- **Test**:
  ```sh
  sleep 5 & jobs
  fg %1
  ```

11) **Segnali e Readline (comportamento interattivo)**
- **Cosa**: `Ctrl-C` → nuova riga/prompt; `Ctrl-\` ignorato; `Ctrl-D` → exit su riga vuota.
- **Come**: handler con singola variabile globale; reset segnali nei child prima di `execve`; integra con Readline.

12) **Qualità, errori e memoria**
- **Cosa**: messaggi d’errore in stile shell (`strerror`); chiudi fd; free deterministici di token/AST/argv; niente leak.
- **Come**: audit: per ogni via felice, verifica fd chiusi e ownership chiara; usa strumenti (valgrind) su Linux.

13) **Modulare: seleziona ≥6 feature**
- **Scelta (le 6 più facili e mirate)**
  1. **Inibitori: doppi/singoli apici e backslash**
     - **Cosa**: gestisci `"..."`, `'...'`, e `\` (escape) nel lexer; rimuovi quote in fase di espansione mantenendo semantiche: in `'...'` niente espansioni; in `"..."` consenti `$VAR`/`${}` ma blocca globbing/split.
     - **Come**: stato di quoting nel lexer; marca token con attributi (`quoted_single`, `quoted_double`); backslash attivo fuori da `'...'` e dentro `"..."`.
     - **Test**: `echo "a b" 'c d' e\ f` → argv coerenti; niente espansioni in `'...'`.

  2. **Espansioni avanzate e tilde**
     - **Cosa**: estendi espansioni a una sotto–lista facile e utile:
       - Tilde: `~`, `~user` (opzionale) → espandi a `HOME` (o home di `user`).
       - Parametri: `${parameter:-word}`, `${parameter:+word}`, `${#parameter}` (lunghezza), `${?}`.
     - **Come**: parser di `${...}` sul token WORD non in `'...'`; valuta secondo ordine: param → tilde → split/globbing (se attivo) → quote removal.
     - **Test**: `echo ${USER:-nobody} ${#HOME} ~`.

  3. **Command substitution `$()`**
     - **Cosa**: sostituisci `$(cmd)` con l’output (stdout) di `cmd` catturato come stringa nel punto di espansione.
     - **Come**: durante espansioni, quando incontri `$(`, parse annidato fino alla `)` bilanciata; esegui in subshell/pipeline, cattura stdout via pipe, trim finale (rispetta quoting: in `"..."` niente globbing).
     - **Test**: `echo $(echo hi)` → `hi`; `echo "x $(printf a\ b)"` → `x a b`.

  4. **Alias (`alias`, `unalias`)**
     - **Cosa**: mappa `name -> replacement` con espansione testuale a inizio comando.
     - **Come**: tabella alias (hash map); espandi prima del parsing del comando (non dentro pipeline già tokenizzata); evita ricorsione infinita (limite espansioni); non espandere dopo token come `(` o dopo redir immediate.
     - **Built-in**:
       - `alias` (senza opzioni): `alias` → lista; `alias name=value` → definizione.
       - `unalias name` → rimozione.
     - **Test**: `alias ll="ls -la" ; ll`.

  5. **Globbing di base (`*`, `?`, `[class]`)**
     - **Cosa**: espandi pattern nel CWD a lista di path ordinati; se nessun match, lascia letterale (POSIX–like).
     - **Come**: matcher semplice: `*` → qualunque sequenza; `?` → un carattere; `[abc]` → set; `\` per escape; integrazione post–param espansion e prima di quote removal.
     - **Test**: `echo *.c ??.h [a-z]*`.

  6. **Tabella hash + built-in `hash`**
     - **Cosa**: cache per lookup di comandi nel `PATH`.
     - **Come**: all’esecuzione di un comando, risolvi e memorizza `name -> /abs/path`; invalida cache su `PATH` variato; `hash` mostra la tabella, `hash -r` (opzionale) la svuota.
     - **Test**: `hash ; ls ; hash` (dovrebbe mostrare `ls` risolto).

- **Come**: implementa ciascuna in moduli separati (`expansion/`, `alias/`, `globbing/`, `hash/`); abilita progressivamente e testa in isolamento; mantieni coerenza POSIX quando in dubbio.

---

**Mini–roadmap per le 6 feature**
- **A) Lexer/expansions groundwork (Inibitori + Tilde/Param)**
  - A1: Stato quote/backslash nel lexer; token attribuiti.
  - A2: Espansore: `${parameter:...}`, `~`, `${?}`, ordine e quote removal.
- **B) Command substitution**
  - B1: Parser per `$()` con bilanciamento; AST figlio.
  - B2: Esecutore: subshell + pipe; cattura stdout.
- **C) Alias**
  - C1: Hash map alias; espansione pre–parse; limite ricorsione.
  - C2: Built-in `alias`/`unalias`.
- **D) Globbing**
  - D1: Matcher `*`, `?`, `[class]`; integrazione nel flusso di espansioni.
  - D2: Ordinamento risultati; fallback letterale.
- **E) Hash table**
  - E1: Cache PATH; invalidazione su `PATH` mutato; built-in `hash`.
  - E2: Uso in exec per velocizzare lookup.

---

**Riuso da Minishell (mapping pratico)**
- `Minishell/src/handle_quote.c` → regole quote/escape: riutilizza e amplia per `${}`.
- `Minishell/src/pipe_shell.c`, `Minishell/src/pipex_utils.c` → pipeline e `dup2`.
- `Minishell/src/heredoc.c` → base per `<<` e regole di espansione su delimiter.
- `Minishell/src/execve.c` → PATH lookup e `execve`.
- `Minishell/src/signal.c` → handler compatibili con Readline.
- `Minishell/src/wildcards.c` → globbing (estendi per `?`, `[]`, `!`).
- Built-in (`echo`, `cd`, `pwd`, `export`, `unset`, `env`) → porta in 42sh e aggiungi `type`, `set`.

Adatta le firme e centralizza le API in header comuni (`env.h`, `parser.h`, `exec.h`, `builtins.h`).

---

**Test di validazione (rapidi)**
- **Precedenze**:
  ```sh
  ls doesnotexist . 2>&1 >/dev/null
  ls doesnotexist . >/dev/null 2>&1
  ```
- **Logici/sequenze**:
  ```sh
  false && echo NO || echo YES ; echo DONE
  ```
- **Subshell**:
  ```sh
  (echo a ; echo b) | wc -l
  ```
- **Espansioni**:
  ```sh
  echo $HOME ${?}
  HOME=/tmp cd ; pwd
  echo ${USER}
  ```
- **Heredoc**:
  ```sh
  cat <<'EOF'\n$HOME\nEOF
  ```

---

**Linee guida di qualità (bonus considerati solo se codice chiaro)**
- Nomi di funzioni/variabili espliciti; niente `ft_parse1`/`ft_parse2`.
- Uso giudizioso di `const`.
- Storico git pulito con commit espliciti.
- Test automatizzati per parser/expansioni/executor.

---

**Cosa consegnare**
- `42sh` eseguibile.
- `Makefile` con regole classiche (`all`, `clean`, `fclean`, `re`, `bonus` se usi bonus).
- `libft/` inclusa e compilata dal `Makefile` del progetto.
- Codice senza crash e senza memory leak.

Con questa roadmap puoi procedere in modo incrementale, validando ogni milestone con test specifici e mantenendo la stabilità come priorità assoluta. Se vuoi, posso preparare gli scheletri dei moduli e una bozza di `Makefile` per 42sh.
