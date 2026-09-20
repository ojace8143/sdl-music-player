#!/bin/sh
# ============================================================================
#  setup-git.sh - push to BOTH GitHub AND Forgejo with a single `git push`
#
#  WHAT IT DOES
#  ------------
#  For each repo you point it at, it attaches TWO push URLs to the existing
#  "origin" remote. Fetching still comes from GitHub; pushing goes to both
#  GitHub and your Forgejo server. It also sets your git identity once and
#  stores your Forgejo token in ~/.git-credentials (git's credential helper).
#
#  AFTER SETUP, plain "git push" (or "git push origin main") sends to BOTH.
#  Nothing extra to type -- every push is a dual push, forever.
#
#
#  FIRST-TIME SETUP (one time per machine)
#  ---------------------------------------
#   1. Get the script onto the machine:
#        scp jcnuc:~/setup-git.sh .            (or git clone the setup-git repo)
#   2. Create a token for THIS machine in Forgejo:
#        http://jcnuc:3000 -> Settings -> Applications -> Generate New Token
#        Name it after the machine (e.g. "jcgentoo") so you can revoke it
#        individually. Scope: write:repository.
#      (Give every machine its own token; revoking one never affects the rest.)
#   3. Make the token file so you never type it again:
#        printf '%s\n' '<your token>' > ~/.forgejo-token && chmod 600 ~/.forgejo-token
#   4. Run it. From inside one repo to configure just that repo:
#        sh setup-git.sh
#      ...or pass several repo paths to do them all at once:
#        sh setup-git.sh ~/code/repo1 ~/code/repo2 ~/code/repo3
#
#
#  ADDING A REPO LATER (no token needed, dotfile handles it)
#  ---------------------------------------------------------
#        sh setup-git.sh ~/code/some-new-repo
#
#
#  HOW DUAL-PUSH WORKS (why this is safe)
#  -------------------------------------
#   Git lets a single remote carry multiple "pushurl" values. origin keeps its
#   normal URL for FETCHING (so you still get updates from GitHub), and has
#   TWO pushurls: one GitHub, one Forgejo. A bare `git push` is "push to every
#   pushurl of the remote", so one command -> both servers. Fetching is never
#   duplicated and stays exactly as it was.
#
#   Repos that were NOT run through this script are untouched -- this only
#   modifies the repos and machine you point it at.
#
#
#  FAQ / TROUBLESHOOTING
#  ---------------------
#   Q: One of the two servers is down. What happens?
#   A: The push fails on that leg (GitHub or Forgejo) and git reports an
#      error. Both must be reachable for a successful dual push. Nothing is
#      half-written -- any refs that already went out are harmless because a
#      later push will just fast-forward/update them again.
#
#   Q: "Could not resolve host: jcnuc"?
#   A: You ran this ON the Forgejo host. "jcnuc" only resolves from OTHER
#      machines on your LAN. Re-run with:  FORGEJO_HOST=localhost:3000
#
#   Q: "Authentication failed for 'https://github.com/...'"
#   A: That's your GitHub auth, which this script does not touch. Make sure
#      the machine already has GitHub working on its own (SSH key, or HTTPS
#      with its own credential helper) before you run this script.
#
#   Q: How do I undo this on a repo?
#   A:   git config --unset-all remote.origin.pushurl
#      Puts it back to normal single-target behavior (GitHub only).
#
#   Q: My token leaked, or I lost a machine. How do I lock it out?
#   A: In Forgejo: Settings -> Applications -> Delete that machine's token.
#      Also delete the line containing it in ~/.git-credentials on that box.
#      GitHub side: rotate the GitHub PAT the same way on GitHub.
#
#   Q: Where exactly is my token stored?
#   A: ~/.git-credentials (plaintext, chmod 600) -- and optionally the
#      ~/.forgejo-token dotfile. Both are read only by your user. A narrow,
#      per-machine token plus chmod 600 is the accepted trade-off here.
#
#   Q: A failed push wiped the line out of my ~/.git-credentials!
#   A: Yes - git runs the credential helper's "erase" operation after an auth
#      failure (wrong token, 401, etc.), and the store helper honors that by
#      deleting the matching line. It's one line and easy to restore: re-run
#      this script (it re-adds the line), or append it by hand:
#        printf 'http://USER:TOKEN@HOST\n' >> ~/.git-credentials
#      The script's self-test also re-verifies the lookup after writing, so
#      you'll see "credential verified" before you ever push.
#
#
#  ENVIRONMENT (all optional, shown with defaults)
#  -----------------------------------------------
#   GITHUB_USER   GitHub owner    (default: ojace8143)
#   FORGEJO_USER  Forgejo user    (default: ojace8143)
#   FORGEJO_HOST  Forgejo host    (default: jcnuc:3000)
#   FORGEJO_TOKEN Forgejo token   (rarely needed; default: ~/.forgejo-token)
#   GIT_EMAIL     Your email, set once if git user.email is unset
# ============================================================================
set -eu

GH_USER="${GITHUB_USER:-ojace8143}"
FJ_USER="${FORGEJO_USER:-ojace8143}"
FJ_HOST="${FORGEJO_HOST:-jcnuc:3000}"
FJ_TOKEN="${FORGEJO_TOKEN:-}"

# Token resolution order: FORGEJO_TOKEN env var, else ~/.forgejo-token dotfile.
# Create the dotfile once per machine:
#   printf '%s\n' '<your token>' > ~/.forgejo-token && chmod 600 ~/.forgejo-token
if [ -z "$FJ_TOKEN" ] && [ -f "$HOME/.forgejo-token" ]; then
    FJ_TOKEN=$(tr -d '\r\n ' < "$HOME/.forgejo-token")
fi

# --- git identity (only set if missing, so existing config wins) ---
if ! git config --global user.name >/dev/null 2>&1; then
    git config --global user.name "$FJ_USER"
    echo "git identity: user.name -> $FJ_USER"
fi
if [ -n "${GIT_EMAIL:-}" ] && ! git config --global user.email >/dev/null 2>&1; then
    git config --global user.email "$GIT_EMAIL"
    echo "git identity: user.email -> $GIT_EMAIL"
fi

# --- Forgejo credentials via git credential store ---
# "store" helper caches creds in ~/.git-credentials so pushes never prompt.
git config --global credential.helper store
if [ -n "$FJ_TOKEN" ]; then
    CREDS="$HOME/.git-credentials"
    touch "$CREDS"
    chmod 600 "$CREDS"
    LINE="http://$FJ_USER:$FJ_TOKEN@$FJ_HOST"
    if grep -qsF "$FJ_HOST" "$CREDS"; then
        echo "credentials for $FJ_HOST already stored in $CREDS"
    else
        printf '%s\n' "$LINE" >> "$CREDS"
        echo "stored Forgejo credentials in $CREDS (chmod 600)"
    fi
else
    echo "no Forgejo token found (FORGEJO_TOKEN or ~/.forgejo-token) - skipping credential store; first push will prompt for username/token"
fi

# Self-test: resolve the credential WITHOUT prompting, so we know a push will
# be password-free. GIT_TERMINAL_PROMPT=0 turns any prompt into a hard fail.
if [ -n "$FJ_TOKEN" ] && printf 'protocol=http\nhost=%s\nusername=%s\n\n' "$FJ_HOST" "$FJ_USER" \
    | GIT_TERMINAL_PROMPT=0 git credential fill 2>/dev/null | grep -qs '^password='; then
    echo "credential verified: git resolves $FJ_USER@$FJ_HOST without prompting"
else
    echo "WARN: git credential fill could not resolve $FJ_USER@$FJ_HOST - first push may prompt for a password (type the TOKEN, not your login password)"
fi

# --- repos ---
# Positional args are repo paths; with no args, configure the current dir.
if [ "$#" -eq 0 ]; then set -- .; fi
for dir in "$@"; do
    dir=$(cd "$dir" 2>/dev/null && pwd) || { echo "skip (bad path): $dir"; continue; }
    name=$(basename "$dir")
    if ! git -C "$dir" rev-parse --git-dir >/dev/null 2>&1; then
        echo "skip (not a repo): $dir"; continue
    fi
    origin=$(git -C "$dir" remote get-url origin 2>/dev/null || true)
    if [ -z "$origin" ]; then
        echo "skip $name (no origin remote)"; continue
    fi

    # Preserve whatever GitHub URL the machine already uses (SSH or HTTPS).
    if printf '%s' "$origin" | grep -q 'github.com'; then
        gh_url="$origin"
    else
        gh_url="https://github.com/$GH_USER/$name.git"
    fi
    fj_url="http://$FJ_USER@$FJ_HOST/$FJ_USER/$name.git"

    # (Re)set pushurls to exactly [GitHub, Forgejo]; fetching stays origin.
    git -C "$dir" config --unset-all remote.origin.pushurl 2>/dev/null || true
    git -C "$dir" config --add remote.origin.pushurl "$gh_url"
    git -C "$dir" config --add remote.origin.pushurl "$fj_url"

    echo "configured $name:"
    git -C "$dir" remote get-url origin
    git -C "$dir" remote get-url --push origin
done