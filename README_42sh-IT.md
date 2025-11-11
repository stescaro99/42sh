# 42sh — Piano di implementazione e guida (IT)

Questo README ti dà una struttura concreta e una sequenza di passi per realizzare 42sh, con particolare attenzione al parsing. È allineato ai subject di 42sh e Minishell e sfrutta le strutture già presenti in `src/sh42.h` (`t_env`, `t_memory`, `t_alias`, `t_line`, `t_data`).

Collega: `subject_it.md` per il testo integrale del subject.

## Obiettivi principali

- Shell interattiva stabile, senza crash e senza memory leak
- Parsing robusto con precedenze corrette (redir < pipe < &&/|| < ;) e gestione quote
- Esecuzione con pipeline, redirezioni, aggregazioni FD, separatori `;`, logici `&&`, `||`, subshell `(...)`, heredoc `<<`
- Built-in: `cd`, `echo`, `exit`, `type` (e gli altri minimi per Minishell, se vuoi compatibilità)
- Variabili (interne + env), export/unset, espansioni `${}`, `$?`, globbing (modulare)
- Job control (`jobs`, `fg`, `bg`, &)
- Segnali e comportamento readline

## Struttura dei moduli

- `src/lexer/` — Tokenizzazione: quote, operatori, parole, numeri, identificatori, FD
- `src/parser/` — Costruzione di AST/plan (usa `t_line` come nodo/segmento)
- `src/expansion/` — Quote removal, var/param espansioni, tilde, globbing
- `src/executor/` — Esecuzione: pipe/redi/aggregazioni, subshell, logici, `;`
- `src/builtins/` — `cd`, `echo`, `exit`, `type`, `export`, `unset`, `set`, `jobs`...
- `src/env/` — Gestione env e variabili interne (`t_env`, `t_memory`)
- `src/alias/` — Alias (`t_alias`) [modulare]
- `src/job/` — Job control e gestione processi in background
- `src/signal/` — Setup/restore handler segnali compatibili con readline
- `src/ui/` — Prompt, integrazione `readline`, history
- `src/utils/` — Helpers (string, array, error, fd, path, gc)

Suggerimento naming file: `lexer.c`, `lexer_token.c`, `parser.c`, `parser_ast.c`, `expand_var.c`, `expand_quote.c`, `exec_pipeline.c`, `exec_redir.c`, `exec_subshell.c`, `builtins_*.c`, `env_*.c`, `job_*.c`.

## Strutture dati (aggancio con sh42.h)

- `t_env`: lista env/variabili con `unsetted` per gestione export/unset
- `t_memory`: key-value per variabili interne (non esportate), usata nelle espansioni
- `t_alias`: mappa alias (modulare)
- `t_line`: rappresenta un’unità della command line/AST semplificato:
  - `type` (short): definire enum per tipo nodo
  - `line`: testo del segmento (o comando normalizzato)
  - `logic` (short): relazione con `next` (NONE/SEQ/AND/OR/PIPE?)
  - `exit`: exit status del segmento (riempito in esecuzione)
  - `parenthesis`: puntatore ad una lista figlia per i `(...)` (subshell / group)
  - `next`: collegamento al prossimo nodo allo stesso livello
- `t_data`: radice del runtime (env, memoria, alias, linea/AST, fds globali)

Proposta enum (in un header comune p.es. `parser_types.h`):
- `t_node_type { NODE_CMD, NODE_REDIR, NODE_PIPE, NODE_GROUP, NODE_EMPTY }`
- `t_logic { LOG_NONE, LOG_SEQ, LOG_AND, LOG_OR }`

Nota: per tenere la precedenza, conviene modellare `PIPE` come relazione tra comandi (non come `logic`), oppure usare una lista interna nel nodo pipeline. Con l’attuale `t_line`, soluzione pratica: tenere `logic` per `;`, `&&`, `||`, e gestire le pipe dentro `line` o con un array di segmenti nel nodo.

## Pipeline di parsing (fasi e contratti)

1) Normalizzazione input
- Usa `readline` per ottenere la riga
- Gestisci spazi, tab, line continuation se necessario

2) Lexing (tokenizzazione)
- Output: `vector<TOKEN>` ordinati, con posizione e tipo
- Token principali:
  - WORD (con quote già gestite ma conservando info per espansioni)
  - OP: `|`, `;`, `&&`, `||`, `<`, `>`, `>>`, `<<`, `<&`, `>&`
  - FD: numeri che precedono redir (`2>`, `2>&1`)
  - PAREN: `(`, `)` per subshell/gruppi
- Regole quote:
  - `'...'`: blocca tutte le espansioni, conserva testo letterale
  - `"..."`: blocca globbing e split, ma consente `$VAR` e `$?`
  - backslash `\` come escape (fuori e in doppi apici), non dentro apici singoli
- Errori lessicali: quote non chiuse, token invalidi → messaggio + status 2

3) Pre-espansione mirata per `<<` (heredoc)
- Determina se il delimitatore è quotato: se sì, disabilita espansioni all’interno dell’heredoc

4) Parsing (con precedenza corretta)
- Grammar ad alto livello (EBNF semplificata):
  - line := list ( ';' list )*
  - list := and_or ( (';'?) and_or )*
  - and_or := pipeline ( ('&&' | '||') pipeline )*
  - pipeline := command ( '|' command )*
  - command := [prefix]* simple_cmd [suffix]* | subshell | assignment_cmd
  - redir := [FD] ( '>' | '>>' | '<' | '<<' | '<&' | '>&' ) WORD
  - prefix/suffix := redir | assignment
  - subshell := '(' line ')'
- Costruzione `t_line`:
  - Un nodo per ogni pipeline/command group al livello di `;`, `&&`, `||` → usa `logic` tra nodi
  - All’interno del nodo `line`, memorizza:
    - argv normalizzati (dopo quote removal ma prima del globbing se modulare)
    - redirezioni aggregate (sinistra/destra, FD sorgente/destinazione)
    - pipeline: elenco di comandi + redir locali
  - Per `(...)`, usa `parenthesis` con una lista figlia di `t_line` (subshell)

5) Espansioni (ordine consigliato)
- Parameter expansion `$VAR`, `${VAR}`, `${?}` su token WORD non in single-quote
- Tilde expansion `~` se all’inizio di WORD non quotata
- Field splitting (se previsto) e pathname expansion (globbing) [modulare]
- Quote removal finale (mantieni i contenuti già espansi)

6) Validazioni finali
- Redirezioni coerenti (FD corretti, file accessibili al momento giusto)
- Esempio di precedenza da testare: `ls x 2>&1 >/dev/null` vs `ls x >/dev/null 2>&1`

## Esecuzione (piano)

- Separatori/logici
  - Percorri lista `t_line` in ordine; valuta `LOG_AND/LOG_OR` usando `exit` del nodo precedente
- Pipeline
  - Crea N-1 pipe, forka ogni processo, duplica STDIN/STDOUT per collegare il flusso
- Redirezioni & aggregazioni
  - Prima di exec, applica ciascuna redir (open, dup2, close); aggregazioni come `2>&1`, `<&0`
- Subshell `(...)`
  - Forka, costruisci un contesto `t_data` figlio (env/mem/alias copiati o condivisi secondo semantica), esegui la lista figlia
- Built-in
  - Esegui nel processo principale se modifica lo stato della shell (`cd`, `export`, `unset`, `exit`)
  - Altrimenti, puoi eseguirlo in child quando dentro una pipeline
- Ricerca eseguibile
  - PATH lookup (cache opzionale: hash table) → `execve`

## Segnali e Readline

- In interattivo:
  - Ctrl-C: interrompe la riga corrente, stampa prompt nuovo (non uccidere la shell)
  - Ctrl-D: EOF → esci se riga vuota
  - Ctrl-\: ignorato
- Imposta handler minimo con una sola variabile globale `volatile sig_atomic_t g_sig` per il numero di segnale (compatibilità Minishell)
- Ripristina i segnali default nei child prima di `execve`

## Variabili ed espansioni

- `t_env` + `t_memory`:
  - `t_memory`: variabili interne (`name=value`) non esportate
  - `export`: promuove da `t_memory` a `t_env`
  - `unset`: marca `unsetted` o rimuove
- Espansioni:
  - `$VAR`, `${VAR}`, `${?}` (exit code)
  - Heredoc: espansioni disabilitate se delimitatore quotato

## Job control (sintesi)

- `&` lancia in background → non bloccare il main loop
- Mantieni tabella job, stati, segnali, built-in `jobs`, `fg`, `bg`

## Piano di lavoro (milestone e check)

1) Boot minimale
- `main.c`: loop readline + stampa token di test + exit su EOF
- Setup segnali base

2) Lexer stabile
- Implementa quote, operatori, FD, parentesi, errori
- Test con suite di stringhe (vedi sezione test)

3) Parser con precedenza
- Implementa grammar e costruzione `t_line`
- Supporta `;`, `&&`, `||`, `|`, redir e `(...)`

4) Espansioni minime
- `$VAR`, `$?`, tilde, quote removal
- Heredoc: riconoscimento delimitatore quotato

5) Esecutore base
- Comando semplice + PATH + redirezioni
- Pipeline
- Logici e `;`
- Subshell

6) Built-in core
- `echo`, `cd`, `type`, `exit`; opzionali: `export`, `unset`, `set`

7) Job control & segnali completi
- `&`, `jobs`, `fg`, `bg` e segnali su processi

8) Pulizia, error handling, no leaks
- Audit fd/pipe, free AST/array/token, verifiche con valgrind

## API suggerite (prototipi indicativi)

Parsing/lexing:
- `int lex_line(const char *in, t_vec_token *out, t_err *e);`
- `int parse_tokens(const t_vec_token *tk, t_line **ast, t_err *e);`
- `void free_ast(t_line *root);`

Espansioni:
- `int expand_command(t_data *sh, t_cmd *cmd, t_err *e);`
- `int expand_heredoc_line(t_data *sh, t_hd_ctx *ctx, char **line);`

Esecuzione:
- `int exec_line(t_data *sh, t_line *ast);`
- `int exec_pipeline(t_data *sh, t_pipeline *pl);`

Env/Mem:
- `int set_internal(t_data *sh, const char *name, const char *value);`
- `int export_var(t_data *sh, const char *name);`
- `int unset_var(t_data *sh, const char *name);`

Nota: i tipi `t_vec_token`, `t_cmd`, `t_pipeline`, `t_err` sono da definire nei nuovi moduli; mantieni lo stile semplice come in `sh42.h` (short al posto di enum se vuoi restare coerente). Le funzioni esistenti in `sh42.h` per env possono essere riusate/estese.

## Parsing: test rapidi (happy path + edge)

- Spazi/quote
  - `echo "a b" 'c d' e\ f`
- Redirezioni/aggregazioni
  - `cmd >out`, `cmd 2>err`, `cmd >>out`, `cmd <in`, `ls x 2>&1 >/dev/null` vs `ls x >/dev/null 2>&1`
- Pipeline
  - `cat a | grep x | wc -l`
- Logici/sequenze
  - `false && echo NO || echo YES ; echo DONE`
- Subshell
  - `(echo a ; echo b) | wc -l`
- Heredoc
  - `cat <<EOF` con delimitatore quotato/non quotato
- Espansioni
  - `echo $HOME ${?}`; con single/double quote

Criteri di successo: AST coerente con precedenza; esecuzioni restituiscono gli exit code attesi; nessun crash con input malformato; nessun leak nelle vie felici.

## Makefile e layout

- Regole minime: `$(NAME)`, `all`, `clean`, `fclean`, `re`, `bonus`
- Compilazione `libft` prima del progetto; link con `-lreadline -lncurses -ltermcap` dove richiesto
- Flag: `-Wall -Wextra -Werror`; opzione debug separata facoltativa

## Error handling & logging

- Errori di parsing: messaggio con caret/posizione se possibile
- Errori di open/dup/exec: messaggio in stile shell (usa `strerror`) e exit status coerenti
- Niente terminazioni inattese (segfault, double free)

## Memoria

- Tracciazione puntuale delle allocazioni di token/AST/argv
- Free deterministici post-esecuzione comando/gruppo
- Evita di memorizzare puntatori in più strutture senza ownership chiara

## Consigli pratici

- Implementa e stabilizza prima `;`, pipe e redir; poi estendi a `&&`, `||`, `(...)`
- Mantieni il parser privo di side-effect sull’ambiente; l’esecutore applica le modifiche
- Sii disciplinato nel separare fasi: lex → parse → expand → exec

---

Con questo piano puoi procedere modulo per modulo, con test mirati soprattutto sul parsing e sulla precedenza degli operatori. Se vuoi, posso generare gli scheletri dei moduli e i file header per partire subito.