# macOS support: plan

Status: **works over USB on one Intel Mac.** Tested on one Mac (Intel, macOS 26) with the
Evernote Edition attached. Stages A and C are done; B1 passed; most of B2 is open.

Done and checked on that Mac:

- `install.sh` runs through and loads both launchd agents.
- The script runs on the stock Python 3.9 and reads the settings file.
- Without a scanner, the watcher with `-w` logs once and waits at 0% CPU; without `-w` it exits 2
  as before.
- SANE finds the scanner without configuration (`fujitsu:ScanSnapiX500EE`).
- Two real scans by button press (2026-11-23): a two-sided flyer and a two-sided business card.
  Both were scanned, queued, processed by the kickstarted worker and saved with sensible names;
  cropping, card detection, orientation of printed sides and OCR look right. The raw frames
  were moved to `done/` and the worker exited 0.
- A button press with no paper reports "Scan failed: no pages" and the watcher carries on.
- Lid closed and reopened: the watcher finds the scanner again (reported by the user).
- The plain `osascript` notifications appear (reported by the user).

Not done yet:

- The rest of B2: lid closed mid-processing, sleep and wake, logout with scans queued, a double
  feed, a legal-length sheet, a stack of cards.
- Comparing processed output with Linux on the same raw frames (A6 step 2).
- `terminal-notifier` and click-to-open.
- Nothing has been run on Linux since the changes, or on Apple Silicon.

Homebrew trouble met on the Intel Mac, for whoever installs next: `/usr/local/libexec` did not
exist and `/usr/local` belongs to root, so every formula's post-install step failed silently.
The visible symptom was `gobject-introspection` failing in pip with a missing certificate file.
Fix: `sudo install -d -o "$USER" -g admin /usr/local/libexec`, then `brew postinstall` for the
installed formulae.

Most of the project should carry over. The processing script (`bin/scan-to-pdf`) is plain Python
calling tools that all exist in Homebrew. What has to be replaced is the Linux plumbing around it:
how the button watcher starts and stops, notifications, and the installer.

The port must work on both Intel and Apple Silicon Macs. The Linux behaviour must stay unchanged.

## Intel and Apple Silicon

The two differ in three ways that matter here. Nothing else in the plan is architecture-specific.

| | Apple Silicon | Intel |
|---|---|---|
| Homebrew prefix | `/opt/homebrew` | `/usr/local` |
| Homebrew packages | Prebuilt bottles, minutes | Built from source on current macOS (checked: the install pulls in build tools such as `nasm` and `doxygen`). Expect a long compile; untimed |
| Watcher binary | arm64 | x86_64 |

Rules that follow:

- **Never hardcode a Homebrew path.** The installer asks `brew --prefix` once and writes the result
  into the plists (`PATH`) and the compiler flags. Nothing in the repository contains
  `/opt/homebrew` or `/usr/local`.
- **Build the watcher on the machine it runs on.** `install.sh` compiles it, as on Linux; no
  binary is shipped. It links against Homebrew's `libsane`, so it gets that library's
  architecture. On an Apple Silicon Mac with an Intel Homebrew under Rosetta the two would not
  match: the installer compares the architecture of `libsane.dylib` (`lipo -archs`) with
  `uname -m` and stops with a clear message.
- **Don't depend on Homebrew's Python.** The script runs on the stock `/usr/bin/python3` (3.9, from
  the Xcode command line tools) on both architectures after one small fix (see below). That keeps
  the shebang identical on Linux and both kinds of Mac.

Test on both before calling the Mac supported. Processing and launchd can be tested on either
without a scanner; the scanner itself needs testing on at least one.

## Order of work

The scanner is needed for less than the first version of this plan assumed. Three stages:

1. **Without a scanner** (stage A): script changes, the watcher's waiting mode, plists, installer,
   and a comparison of processed output against Linux.
2. **With the scanner** (stage B): the go/no-go check that SANE can drive it on macOS, then live
   tests.
3. **Document** (stage C).

Stage B is still the go/no-go for the whole port. Stage A is done first only because it can be,
and it is not wasted if B fails late: most of it (Python 3.9 fix, settings file, tool check) also
improves the Linux version.

## Stage A: everything that needs no scanner

### A1. Install the tools

`brew install sane-backends imagemagick tesseract poppler`
(plus `tesseract-lang` for languages other than English). On Intel this is the slow step.

`terminal-notifier` is optional and installed separately. On Intel, Homebrew has to build it from
source and that needs the full Xcode app, not just the command line tools (found when trying it).
Without it, notifications still appear but clicking one does not open the PDF.

### A2. Changes to `bin/scan-to-pdf`

- **Python 3.9.** The script fails at import on the stock Python: `Path | None` in the `Side`
  dataclass (line 149) needs 3.10. Add `from __future__ import annotations`. Keep the
  `#!/usr/bin/python3` shebang.
- **Settings file.** Read `~/.config/scan-button.env` in the script itself, so both systems share
  one mechanism (values already in the environment win). Two constraints:
  - It must be loaded before the first setting is read. Settings are read at import, starting at
    line 35 and again further down, so the loader goes at the very top.
  - The file is in systemd `EnvironmentFile` syntax, not shell: `KEY=value`, `#` comments,
    optional quotes around the value. The parser must accept what systemd accepts, so existing
    files keep working.
- **Worker start.** `start_worker()` crashes on a Mac today: with no `systemctl`, `subprocess.run`
  raises `FileNotFoundError`, so the detached-worker fallback is never reached. On macOS run
  `launchctl kickstart gui/<uid>/com.aiex.scan-process` instead, and fall back to the detached
  worker if that fails (agent not installed). Catch the missing-`systemctl` case on Linux too.
- **`notify()`.** On macOS use `terminal-notifier` if installed, else a plain `osascript`
  notification (no click-to-open). Keep `notify-send` on Linux. Click-to-open: `-open` takes a
  URL, so pass `file://…`. There is no equivalent of "critical" urgency; drop it on macOS.
- **Tool check.** At startup, name any missing tool (`scanimage`, `magick`, `tesseract`,
  `pdftotext`, `pdfunite`) instead of failing somewhere in the pipeline.

### A3. Changes to `src/scan-button-watch.c`

On Linux the watcher only exists while the scanner is on USB, and exits when it goes away. On
macOS it is always running (see "When the watcher runs"), so it needs a waiting mode behind a
flag (`-w`). Linux does not pass the flag and behaves as before.

With `-w`, all three exits become "go back to waiting":

- the scanner can't be opened at startup,
- five button reads in a row fail (scanner switched off, Mac woke from sleep),
- the scanner can't be reopened after a scan.

While waiting: retry every few seconds, log once when the scanner goes away and once when it comes
back, not on every attempt. Call `sane_exit()` and `sane_init()` between attempts, so the backend
looks at USB afresh. (The fujitsu backend appears to keep its device list once it has one, and a
scanner that is switched back on gets a new USB address. Believed from memory of the backend, not
checked against its source; the reinit is cheap either way.)

Build: `cc -O2 -Wall -Wextra -I"$prefix/include" -L"$prefix/lib" -o … -lsane` with
`prefix=$(brew --prefix)`.

### A4. launchd agents

Two agents in `~/Library/LaunchAgents`, generated by the installer from templates so the
Homebrew prefix and home directory are filled in:

| | `com.aiex.scan-button` | `com.aiex.scan-process` |
|---|---|---|
| Runs | `scan-button-watch -w -i 1000 -- scan-to-pdf` | `scan-to-pdf --worker` |
| Starts | At login (`RunAtLoad`), kept running (`KeepAlive`) | At login (`RunAtLoad`, picks up leftovers) and on `launchctl kickstart` after each scan |
| Priority | Normal | `Nice` 10, `LowPriorityIO` |
| `PATH` | `<brew prefix>/bin:/usr/bin:/bin:/usr/sbin:/sbin` | same |
| Logs | `~/Library/Logs/scan-button.log` | `~/Library/Logs/scan-process.log` |

Notes:

- **Not `QueueDirectories`.** launchd relaunches such a job for as long as the folder is
  non-empty, and the queue folder always holds `done/`, `failed/` and `.lock`. The worker would
  respawn forever.
- **`PATH` is required.** launchd agents get `/usr/bin:/bin:/usr/sbin:/sbin`, which contains
  neither Homebrew prefix.
- **Logs are required.** There is no journal; without `StandardOutPath`/`StandardErrorPath` the
  output is lost. The watcher and worker log little, so no rotation for now.
- No `EnvironmentFile` equivalent is needed: the script reads the settings file itself (A2).

### A5. Installer

Branch on `uname` in `install.sh`. The macOS branch: check the tools and the architecture match,
build the watcher, link both programs into `~/.local/bin`, write the two plists,
`launchctl bootout` any old copies, `launchctl bootstrap gui/$UID`. No sudo. The Linux branch is
the current script, untouched.

### A6. Test without the scanner

1. **Waiting mode.** With no scanner attached the watcher must sit quietly, log once, and use no
   noticeable CPU. This is exactly the state of a Mac with the scanner lid closed.
2. **Processing.** Copy a raw scan folder from the Linux machine
   (`~/.local/state/scan-queue/done/<one>`), run `scan-to-pdf --from-dir <folder>` and compare the
   PDFs with the Linux output. This tests cropping, splitting, orientation and naming. It also
   answers whether Homebrew's ImageMagick supports `%[minimum-bounding-box:unrotate]` and `%@`,
   which the crop relies on.
3. **Queue.** Drop the same folder into the queue under a timestamp name, `launchctl kickstart`
   the worker, check the PDF, the move to `done/`, and that the worker exits and stays stopped.
4. **Notifications.** Whether `terminal-notifier` still shows notifications and opens the file on
   click, on current macOS and when started from a launchd agent, is unverified. If it doesn't,
   fall back to `osascript` notifications without click-to-open.
5. Repeat 1 to 4 on the other architecture.

## Stage B: with the scanner

### B1. Go/no-go: does SANE drive the scanner on macOS

1. `scanimage -L` with the scanner open.
2. One duplex scan with the options the script uses:
   ```sh
   scanimage -d fujitsu --source "ADF Duplex" --mode Color --resolution 300 \
     --page-height 356 -y 356 --overscan On \
     --df-action Stop --df-thickness=yes \
     --format=tiff --batch=p%04d.tif
   ```
3. `scanimage -d fujitsu -A` should list the `scan` option (the button).

Things that could go wrong:

- **Fujitsu's own software.** ScanSnap Home or ScanSnap Manager, if installed, may hold the scanner
  and would have to be quit or removed.
- **USB access.** SANE uses libusb on macOS. No kernel driver should claim a vendor-class scanner,
  but this is unverified.

Not a risk any more: the Evernote Edition USB ID (`04c5:13f3`). Upstream sane-backends lists it in
`fujitsu.conf` as "ScanSnap iX500EE", next to the regular iX500 (`04c5:132b`). If Homebrew's copy
should lack it, add `usb 0x04c5 0x13f3` to `$(brew --prefix)/etc/sane.d/fujitsu.conf`.

If B1 fails and can't be fixed with configuration, the port stops here.

### B2. Live tests

Button press, lid closed and reopened (the watcher must find the scanner again), lid closed
mid-processing, Mac asleep and awake with the scanner on, logout with scans queued, a double feed,
a legal-length sheet, a batch of business cards.

## Stage C: document

Add a macOS section to the README (requirements, install, where the logs are) and list the Mac as
supported, naming the architectures actually tested.

## When the watcher runs

On Linux the watcher only exists while the scanner is on USB (lid open). macOS has a true
equivalent (launchd `LaunchEvents` with IOKit matching on the USB vendor/product ID), but it is
fiddly to set up and debug.

Decision: an always-on agent with the waiting mode from A3. The cost is one cheap USB probe every
few seconds while the scanner is off. Move to IOKit matching later only if that turns out to
matter.

## Other notes

- **Paths.** The queue lives in `~/.local/state/scan-queue`, which works on a Mac as is. Moving it
  to `~/Library/Application Support` is not worth a platform difference.
- **File locking and threading.** `fcntl.flock` and `ThreadPoolExecutor` behave the same.
- **Permissions.** macOS will ask once to allow notifications. If the output folder is in iCloud
  or another protected location (Documents, Desktop), the background agent may also need Files and
  Folders access. The default, `~/Notes/Scans`, is not protected.
- **`readlink -f`** in `install.sh` exists on macOS 12.3 and later.

## Files this would add or change

- `launchd/com.aiex.scan-button.plist.in`, `launchd/com.aiex.scan-process.plist.in` (new templates)
- `install.sh` (macOS branch)
- `src/scan-button-watch.c` (waiting mode behind `-w`)
- `bin/scan-to-pdf` (Python 3.9, settings file, worker start, notifications, tool check)
- `README.md` (macOS section)

## Size

Stage A is about a day, most of it launchd and testing, plus the Homebrew build time on Intel.
Stage B is an hour if SANE works. If the driver misbehaves on macOS, the effort is open-ended and
the answer may be "not worth it".
