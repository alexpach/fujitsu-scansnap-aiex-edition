#!/usr/bin/env bash
# Install the scan-button watcher.
#  - builds src/scan-button-watch.c (needs a C compiler + sane)
# Linux:
#  - installs the user service (no root)
#  - installs the udev rule that starts/stops the service with the scanner (needs sudo once)
# macOS (Homebrew, Intel or Apple Silicon):
#  - installs two launchd agents in ~/Library/LaunchAgents (no root)
set -euo pipefail
here=$(dirname "$(readlink -f "$0")")
cd "$here"

if [[ $(uname) == Darwin ]]; then
  die() { echo "install.sh: $*" >&2; exit 1; }
  command -v brew >/dev/null || die "Homebrew is required (https://brew.sh)"
  prefix=$(brew --prefix)

  missing=()
  for tool in scanimage magick tesseract pdftotext pdfunite; do
    [[ -x $prefix/bin/$tool ]] || missing+=("$tool")
  done
  ((${#missing[@]} == 0)) || die "not found in $prefix/bin: ${missing[*]}
  brew install sane-backends imagemagick tesseract poppler"
  [[ -x /usr/bin/python3 ]] && /usr/bin/python3 -c pass 2>/dev/null \
    || die "/usr/bin/python3 does not run; install the command line tools: xcode-select --install"
  [[ -x $prefix/bin/terminal-notifier ]] \
    || echo "Note: terminal-notifier is not installed; notifications will not open the PDF when clicked."

  # The watcher is built for this machine and links against Homebrew's libsane, so the two must
  # have the same architecture (an Intel Homebrew under Rosetta on Apple Silicon does not).
  arch=$(uname -m)
  lib=$prefix/lib/libsane.dylib
  [[ " $(lipo -archs "$lib") " == *" $arch "* ]] \
    || die "$lib is built for $(lipo -archs "$lib"), this shell runs as $arch.
  Use the Homebrew that matches this Mac's processor."

  mkdir -p build ~/.local/bin ~/Library/LaunchAgents ~/Library/Logs
  cc -O2 -Wall -Wextra -I"$prefix/include" -L"$prefix/lib" \
    -o build/scan-button-watch src/scan-button-watch.c -lsane
  chmod +x bin/*
  ln -sf "$here/build/scan-button-watch" ~/.local/bin/scan-button-watch
  ln -sf "$here/bin/scan-to-pdf"         ~/.local/bin/scan-to-pdf

  domain=gui/$(id -u)
  for label in com.aiex.scan-button com.aiex.scan-process; do
    plist=~/Library/LaunchAgents/$label.plist
    sed -e "s|@HOME@|$HOME|g" -e "s|@PREFIX@|$prefix|g" "launchd/$label.plist.in" > "$plist"
    plutil -lint -s "$plist"
    # Replace a copy that is already loaded; bootout returns before the job is really gone.
    launchctl bootout "$domain/$label" 2>/dev/null || true
    for _ in 1 2 3 4 5 6 7 8 9 10; do
      launchctl print "$domain/$label" >/dev/null 2>&1 || break
      sleep 0.5
    done
    launchctl bootstrap "$domain" "$plist"
  done

  sleep 1
  launchctl print "$domain/com.aiex.scan-button" | grep -E '^\s*(state|pid|last exit code) =' || true
  echo "Installed. Logs: ~/Library/Logs/scan-button.log and scan-process.log"
  exit 0
fi

mkdir -p build ~/.local/bin ~/.config/systemd/user
cc -O2 -Wall -Wextra -o build/scan-button-watch src/scan-button-watch.c -lsane
chmod +x bin/*
ln -sf "$here/build/scan-button-watch" ~/.local/bin/scan-button-watch
ln -sf "$here/bin/scan-to-pdf"         ~/.local/bin/scan-to-pdf

# Remove the old always-on polling setup if present.
systemctl --user disable --now scan-button.service 2>/dev/null || true
rm -f ~/.local/bin/scan-button-daemon

cp systemd/*.service ~/.config/systemd/user/
systemctl --user daemon-reload
systemctl --user enable scan-button-login.service

if ! cmp -s udev/99-scansnap.rules /etc/udev/rules.d/99-scansnap.rules; then
  echo "Installing udev rule (sudo)…"
  sudo install -m 644 udev/99-scansnap.rules /etc/udev/rules.d/99-scansnap.rules
  sudo udevadm control --reload
fi
# Re-trigger so an already-connected scanner gets picked up now.
sudo udevadm trigger --action=change --attr-match=idVendor=04c5 --attr-match=idProduct=13f3 || true

# Start the watcher now if the scanner is already on (covers a failed/skipped udev trigger).
systemctl --user start scan-button-login.service || true
sleep 1
systemctl --user --no-pager status scan-button.service | head -n 6 || true
