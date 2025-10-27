# 42sh

## Cose da fare (TODO)

Di seguito trovi una checklist organizzata sui requisiti del subject del progetto 42sh. Ogni voce contiene una breve descrizione. Spunta gli elementi man mano che li implementi e testi.

### Parte obbligatoria

- [x] Prompt display — Mostrare un prompt interattivo coerente e personalizzabile.
- [x] Esecuzione comandi e argomenti — Eseguire comandi esterni rispettando gli argomenti e la variabile PATH.
- [x] Gestione spazi e tabulazioni — Parsing corretto di spazi e tab (tokenization affidabile).
- [ ] Redirection e aggregazioni — Supportare gli operatori di redirezione e aggregazione:
  - [x] `>`  — redirect output (truncate)
  - [x] `>>` — redirect output (append)
  - [x] `<`  — redirect input
  - [ ] `<<` — here-document
  - [ ] `>&` — dup stdout/stderr
  - [ ] `<&` — dup stdin
- [x] Pipe `|` — Collegare più processi tramite pipe.
- [x] Separatore `;` — Eseguire comandi separati da `;`.
- [ ] Built-in fondamentali — Implementare i built-in richiesti con comportamento POSIX:
  - [x] `cd` — cambiare directory, gestire `$HOME`, `-` e path relativi/assoluti.
  - [x] `echo` — stampa con gestione opzioni standard (es. `-n`).
  - [x] `exit` — uscita con codice e cleanup.
  - [ ] `type` — mostra se un nome è built-in, esterno o alias.
- [x] Operator logic: `&&` e `||` — Valutazione condizionale della catena di comandi.
- [ ] Precedenza degli operatori — Assicurarsi che `; && ||` e redirezioni/pipes rispettino la precedenza corretta (es. ordini diversi producono output diverso).
- [ ] Variabili interne / ambiente:
  - [x] Creazione variabili interne `name=value`.
  - [x] `export` per promuovere variabili all'ambiente.
  - [ ] `set` per elencare le variabili interne.
  - [x] `unset` per rimuovere variabili interne/ambiente.
  - [ ] Ambiente temporaneo per un singolo comando: `VAR=val cmd`.
  - [x] Espansione semplice `${VAR}` e `${?}` per codice di uscita precedente.
- [ ] Job control — Supportare `jobs`, `fg`, `bg` e l'operatore `&`.
- [x] Gestione segnali — Intercettare e gestire segnali (SIGINT, SIGQUIT, SIGCHLD, ecc.).
- [ ] Opzioni built-in conformi POSIX — Implementare le opzioni richieste per i built-in (salvo eccezioni documentate).

### Parte modulare (scegliere e implementare almeno 6)

- [ ] Inibitori: doppi/singoli apici e backslash (`"`, `'`, `\`) — Gestione corretta delle stringhe e dell'escaping.  # da fare
- [ ] Pattern matching (globbing) — `*`, `?`, `[]`, `!`, intervalli, e escaping `\`.
- [ ] Espansioni avanzate e tilde — `~` e le forme di parametro `${parameter:-word}`, `${parameter:=word}`, `${parameter:?word}`, `${parameter:+word}`, `${#parameter}`, `${parameter%}`, `${parameter%%}`, `${parameter#}`, `${parameter##}`.  # da fare
- [ ] Gruppi di controllo e subshell: `()` e `{}` — Eseguire gruppi di comandi isolati o con redirezioni dedicate.
- [ ] Command substitution `$()` — Sostituire l'output di un comando nella linea.  # forse già fatto
- [ ] Arithmetic expansion `$(())` — Supporto per operazioni aritmetiche (++, --, +, -, *, /, %, <=, >=, <, >, ==, !=, &&, ||).
- [ ] Process substitution `<()` e `>()` — Se possibile, implementare.
- [ ] History completa — `!!`, `!word`, `!number`, `!-number`, salvataggio su file, built-in `fc`, CTRL-R.
- [ ] Completamento contestuale dinamico — Comandi, file, variabili, con filtro contestuale.
- [ ] Modalità di editing Vi e Readline — Implementare scorciatoie richieste e scelta tramite `set -o`.
- [ ] Alias (`alias`, `unalias`) — Gestione di alias utente. # da fare
- [ ] Hash table e `hash` builtin — Cache dei percorsi dei comandi.
- [ ] Built-in `test` completo — Supporto degli operatori file e numerici elencati.

> Nota: scegliere almeno 6 delle voci sopra per raggiungere la parte modulare.

### Note e linee guida
- Testare ogni funzionalità in isolamento e con casi composti (redirezioni + pipe + operatori logici).
- Dare priorità alla stabilità e alla correttezza della grammatica della shell prima di aggiungere feature modulari o bonus.
- Mantenere una storia Git chiara con commit atomici e messaggi espliciti.
- Aggiungere test automatici per le funzionalità critiche.

### Check

Ricontrollare tutto il parsing, l'esecuzione dei comandi di minishell