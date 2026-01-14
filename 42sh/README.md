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

# 42sh — Guida operativa e mappa dei file

Questo README sostituisce e dettaglia la roadmap: ora contiene mappature consigliate file→funzioni, dove mettere ogni sorgente, e raccomandazioni per rispettare la Norminette (max 5 funzioni/file, 25 righe/funzione).

**Prerequisiti e build**
- **Librerie**: `libc`, `readline` (`-lreadline`), `termcap`/`ncurses` se usi funzionalità avanzate del terminale.
- **Libft**: includi `libft/` e compila prima.
- **Flags**: `-Wall -Wextra -Werror`.

Esempio rapido:
```sh
make -C libft re
make
./42sh
```

**Nota Norminette (vincolo obbligatorio)**
- Ogni file `.c` deve contenere al massimo 5 funzioni. Ogni funzione non deve superare 25 righe.
- Strategia: spezza i moduli in più file piccoli (es. `lexer_state.c`, `lexer_tokens.c`) in modo che ogni file resti sotto il limite.
- Conta anche le `static` — sono considerate funzioni ai fini della Norminette. Preferisci collocare helper in file separati.

**Struttura consigliata e cosa mettere in ogni file**
Qui sotto trovi una mappa concreta: per ogni cartella suggerisco file e fino a 5 funzioni per file (nome e comportamento). Rispetta la regola 5 funzioni/file dividendo se necessario.

- `src/main.c`
  - `main(int argc, char **argv, char **env)` — bootstrap: init `t_data`, loop shell, cleanup.

- `src/ui/` (prompt, readline, history)
  - `ui/prompt.c`
    - `char *build_prompt(t_data *d)` — costruisce stringa prompt.
    - `void print_header(void)` — (opzionale) banner iniziale.
  - `ui/readline_integration.c`
    - `char *read_line(t_data *d)` — invoca `readline`, gestisce `add_history`.
    - `void setup_readline(void)` — init readline (completions, keybindings).

- `src/lexer/`
  - `lexer/tokenize.c`
    - `t_token *tokenize(const char *s)` — produce lista di token base.
    - `void free_tokens(t_token *t)` — libera token.
  - `lexer/state.c`
    - `void lexer_init(t_lexer *lx, const char *s)` — init stato lexer.
    - `t_token *lexer_next(t_lexer *lx)` — ritorna token successivo (semplice helper).

- `src/parser/`
  - `parser/line_parser.c`
    - `t_line *parse_line(t_token *tokens)` — parsing ad alto livello (`;`, `&&`, `||`).
    - `void free_line(t_line *line)` — libera struttura AST/minimap.
  - `parser/command_parser.c`
    - `t_cmd *parse_command(t_token **curr)` — costruisce `t_cmd` (argv + redirs).

- `src/expansion/`
  - `expansion/params.c`
    - `char *expand_params(t_data *d, const char *word)` — `${}`, `$?`, `$VAR`.
  - `expansion/quote_remove.c`
    - `char *remove_quotes(const char *s)` — elimina apici/escape lasciando semantica.
  - `expansion/tilde_and_glob.c`
    - `char **apply_globbing(char **words)` — espande `*`, `?`, `[ ]` (opzionale).

- `src/heredoc/`
  - `heredoc/heredoc.c`
    - `int handle_heredoc(t_data *d, const char *delim, bool quoted)` — legge e crea temp file, ritorna fd.

- `src/executor/`
  - `executor/execve_utils.c`
    - `char *resolve_path(t_data *d, const char *name)` — cerca in `PATH` e usa cache.
    - `int launch_builtin(t_data *d, t_cmd *c)` — esegue built-in in main quando necessario.
  - `executor/pipeline.c`
    - `int run_pipeline(t_data *d, t_pipeline *pl)` — esegue pipeline con fork/dup2.

- `src/builtins/`
  - `builtins/cd.c`
    - `int builtin_cd(t_data *d, char **argv)` — cambia dir, aggiorna env `$PWD`/`$OLDPWD`.
  - `builtins/echo.c`
    - `int builtin_echo(char **argv)` — implementa `-n`.
  - `builtins/export_unset.c`
    - `int builtin_export(t_data *d, char **argv)`
    - `int builtin_unset(t_data *d, char **argv)`

- `src/env/`
  - `env/env_list.c`
    - `t_env *env_from_envp(char **envp)` — costruisce lista.
    - `char **env_to_envp(t_env *e)` — serializza per `execve`.

- `src/job/`
  - `job/jobs.c`
    - `int job_add(t_data *d, pid_t pgid, char *cmdline)` — registra job background.
    - `void job_check_children(t_data *d)` — aggiorna stati al ritorno dei figli.

- `src/signal/`
  - `signal/handlers.c`
    - `void setup_signals(void)` — installa handler compatibili con readline.
    - `void sigint_handler(int sig)` — imposta stato globale per prompt.

- `src/utils/`
  - `utils/str_helpers.c` (es. 3–5 funzioni)
    - `char *ft_strjoin_safe(...)`, `char **ft_split_ws(...)`, ecc.
  - `utils/fd_helpers.c`
    - `int safe_dup2(int oldfd, int newfd)` — dup2 con controllo.

Consiglio: per ogni file `.c` mantieni al massimo 5 funzioni pubbliche/`static`. Se una funzione cresce oltre 25 righe, spostane pezzi in un altro file (es. `parser_util.c`) per rispettare la Norminette.

**Esempi pratici di divisione per Norminette**
- Se `tokenize()` è >25 righe: estrai `parse_operator()`, `parse_word()` e mettili in `lexer_token_helpers.c` (ogni helper conta come funzione in quel file). L'obiettivo è che ogni file contenga meno di 5 funzioni MA ogni funzione rimanga chiara e testabile.
- Raggruppa gli helper strettamente correlati nello stesso file solo se il totale resta sotto 5 funzioni.

**Test rapidi**
- Controllo precedenze (es.)
```sh
false && echo NO || echo YES ; echo DONE
```
- Heredoc:
```sh
cat <<'EOF'
$HOME
EOF
```

**Consegna minima richiesta**
- Eseguibile `42sh`.
- `Makefile` (all/clean/fclean/re) che compila `libft/`.
- Codice organizzato per rispettare la Norminette (vedi sopra).

Vuoi che generi scheletri `.c/.h` per le cartelle sopra (es. file vuoti con protoprofili di funzioni) in modo che tu possa implementare rispettando subito la Norminette?
     - **Come**: matcher semplice: `*` → qualunque sequenza; `?` → un carattere; `[abc]` → set; `\` per escape; integrazione post–param espansion e prima di quote removal.
