#!/usr/bin/env bash
# Install the scan-button watcher.
#  - builds src/scan-button-watch.c (needs gcc + sane)
#  - installs the user service (no root)
#  - installs the udev rule that starts/stops the service with the scanner (needs sudo once)
set -euo pipefail
here=$(dirname "$(readlink -f "$0")")
cd "$here"

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
