# macOS support: plan

Status: **plan only, nothing implemented or tested on a Mac.** This comes from reading the code on
Linux. The project currently runs on Arch Linux (see [README.md](README.md)).

Most of the project should carry over. The processing script (`bin/scan-to-pdf`) is plain Python
calling tools that all exist in Homebrew. What has to be replaced is the Linux plumbing around it:
how the button watcher starts and stops, notifications, and the installer.

## Step 1: prove the scanner works at all (go/no-go)

Everything depends on SANE's `fujitsu` driver talking to the iX500 over USB on macOS. Until that
works, nothing else is worth doing.

1. `brew install sane-backends`, then `scanimage -L` with the scanner open.
2. Run one duplex scan with the same options the script uses:
   ```sh
   scanimage -d fujitsu --source "ADF Duplex" --mode Color --resolution 300 \
     --page-height 356 -y 356 --overscan On \
     --df-action Stop --df-thickness=yes \
     --format=tiff --batch=p%04d.tif
   ```
3. Check that the Scan button can be read: `scanimage -d fujitsu -A` should list the `scan` option.

Things that could go wrong here:

- **Fujitsu's own software.** ScanSnap Home or ScanSnap Manager, if installed, may hold the scanner
  and would have to be quit or removed.
- **Evernote Edition USB ID** (`04c5:13f3`; the regular iX500 is `04c5:132b`). The driver may not
  recognise it out of the box. On Linux that took one line in `fujitsu.conf` (see README, Setup);
  the same line should work in Homebrew's `$(brew --prefix)/etc/sane.d/fujitsu.conf`.
- **USB access.** SANE uses libusb on macOS. No kernel driver should claim a vendor-class scanner,
  but this is unverified.

If step 1 fails and can't be fixed with configuration, stop here.

## Step 2: port the plumbing

| Piece | Linux today | Mac replacement | Effort |
|---|---|---|---|
| Button watcher (`src/scan-button-watch.c`) | Small C program against SANE | Same code; build flags need Homebrew paths (`-I$(brew --prefix)/include -L$(brew --prefix)/lib -lsane`) | Small |
| Start watcher when scanner turns on | `udev/99-scansnap.rules` + `scan-button.service` (`BindsTo=dev-scansnap.device`) | A launchd agent in `~/Library/LaunchAgents`, kept running; the watcher waits quietly and retries while the scanner is off | Medium |
| Start at login with scanner already on | `scan-button-login.service` | Not needed; the launchd agent starts at login (`RunAtLoad`) | None |
| Background processing queue | `scan-process.service` (`Nice=10`, idle I/O) | A second launchd agent watching the queue folder (`QueueDirectories`), with `LowPriorityIO` and `Nice` | Small |
| Kick the worker after a scan | `systemctl --user start scan-process.service` in the script | Nothing, if launchd watches the queue folder; otherwise `launchctl kickstart` | Small |
| Notifications with click-to-open | `notify-send --action` + `xdg-open` | `terminal-notifier -open <file>` (Homebrew). Plain `osascript` notifications can't open a file on click | Small |
| Settings file | `EnvironmentFile=` in the units | Have the script read `~/.config/scan-button.env` itself, so both systems share one mechanism | Small |
| Installer | `install.sh` (systemd, udev, sudo) | Branch on `uname`: on a Mac, build, write the two plists, `launchctl bootstrap gui/$UID`. No sudo needed | Medium |

### The one real design decision: when the watcher runs

On Linux the watcher only exists while the scanner is on USB (lid open). macOS has a true
equivalent (launchd `LaunchEvents` with IOKit matching on the USB vendor/product ID), but it is
fiddly to set up and debug.

Recommended start: an always-on agent. The watcher already exits when the scanner disappears; on
the Mac it would instead sleep a few seconds and retry `sane_open`. The cost is one cheap USB probe
every few seconds while the scanner is off. Move to IOKit matching later only if that turns out to
matter.

Needed code change in `scan-button-watch.c`: a `--wait` flag (or similar) that retries opening the
device instead of exiting, without logging on every failed attempt.

### Changes to `bin/scan-to-pdf`

- `notify()`: use `terminal-notifier` on macOS, keep `notify-send` on Linux, stay silent if neither
  is installed (already the behaviour when `notify-send` is missing).
- Worker start: skip the `systemctl` call on macOS.
- Read `~/.config/scan-button.env` at startup (values already in the environment win).
- Startup check that names any missing tool (`scanimage`, `magick`, `tesseract`, `pdftotext`).

## Step 3: check the processing script

- **Tools.** ImageMagick 7, Tesseract (plus `tesseract-lang` for other languages) and poppler are
  all in Homebrew. Verify the ImageMagick build supports `%[minimum-bounding-box:unrotate]` and
  `%@`, which the crop relies on.
- **Paths.** The queue lives in `~/.local/state/scan-queue`, which works on a Mac as is. Moving it
  to `~/Library/Application Support` is optional and not worth a platform difference.
- **File locking and threading.** `fcntl.flock` and `ThreadPoolExecutor` are standard Python and
  behave the same.
- **PATH under launchd.** Agents get a minimal PATH without `/opt/homebrew/bin`. Set
  `EnvironmentVariables` → `PATH` in both plists.
- **Permissions.** macOS will ask once to allow notifications. If the output folder is in iCloud
  or another protected location (Documents, Desktop), the background agent may also need Files and
  Folders access.
- **Python.** The script needs Python 3 with only the standard library. Use Homebrew's `python3`
  rather than relying on the Xcode command line tools copy.

## Step 4: test and document

1. Reprocess a saved raw scan on the Mac with `scan-to-pdf --from-dir <folder>` (copy one from
   `~/.local/state/scan-queue/done/` on the Linux machine) and compare the PDFs with the Linux
   output. This tests cropping, splitting, orientation and naming without the scanner.
2. Live tests: button press, lid closed mid-processing, logout with scans queued, a double feed,
   a legal-length sheet, a batch of business cards.
3. Add a macOS section to the README (requirements and install) and list the Mac as supported.

## Files this would add or change

- `launchd/com.aiex.scan-button.plist`, `launchd/com.aiex.scan-process.plist` (new)
- `install.sh` (macOS branch)
- `src/scan-button-watch.c` (retry-instead-of-exit mode)
- `bin/scan-to-pdf` (notifications, worker start, settings file, tool check)
- `README.md` (macOS section)

The Linux behaviour must stay unchanged.

## Size

If step 1 passes cleanly, about a day of work, most of it launchd and testing. If the SANE driver
misbehaves on macOS, the effort is open-ended and the answer may be "not worth it".
