# Fujitsu ScanSnap - AIex Edition

Press the Scan button on a Fujitsu ScanSnap iX500 under Linux or macOS and get named, searchable PDFs in a folder. No vendor software, no cloud: everything runs locally with SANE, ImageMagick and Tesseract.

What a button press does:

- scans the stack (duplex, 300 dpi color) and queues it, so you can scan the next stack right away
- crops and straightens each sheet, drops blank sides, turns pages the right way up
- splits a stack into separate documents and saves each business card as its own file
- puts the front of a business card first: the side with the contact details, or the English side of a two-language card
- OCRs everything and names the files from their content (`2026-09-17 Acme Health statement.pdf`, `Business card - Jane Doe (Acme).pdf`)
- never overwrites a file; rescans of the same paper go to `duplicates/`
- shows a notification you can click to open the result

Nothing runs while the scanner is off: a udev rule starts the button watcher when the lid opens.

## Requirements

- Fujitsu ScanSnap iX500. Developed on the **Evernote Edition** (USB ID `04c5:13f3`). The regular iX500 is `04c5:132b`: change `idProduct` in `udev/99-scansnap.rules`.
- Linux with systemd (user services) and a notification daemon. Developed on Omarchy (Arch).
- Packages: `sane`, `imagemagick`, `tesseract` + `tesseract-data-eng`, `poppler`, `libnotify`, `python` (3.10+), `gcc`.

## Install

```sh
./install.sh        # builds the button watcher, installs the user services and the udev rule (asks for sudo once)
```

PDFs go to `~/Notes/Scans`; set `SCAN_OUTDIR` in `~/.config/scan-button.env` to change that. More settings are listed under "How it works".

### macOS

Works over USB on an Intel Mac (macOS 26) with the Evernote Edition; Apple Silicon is untested. See [MAC-SUPPORT.md](MAC-SUPPORT.md) for what has and hasn't been checked.

```sh
brew install sane-backends imagemagick tesseract poppler   # on Intel this compiles from source: hours
./install.sh        # builds the button watcher, installs two launchd agents (no sudo)
```

- Needs Homebrew and the Xcode command line tools (for `cc` and `/usr/bin/python3`). No Fujitsu software; if ScanSnap Home is installed, quit it.
- The button watcher runs all the time and waits quietly while the scanner is off.
- Logs: `~/Library/Logs/scan-button.log` and `~/Library/Logs/scan-process.log`.
- Notifications: with `terminal-notifier` installed, clicking one opens the PDF; without it they are plain. (On Intel, Homebrew can only build `terminal-notifier` with the full Xcode app.)
- Other OCR languages: `brew install tesseract-lang`.
- To release the scanner for another app or for cleaning: `launchctl bootout gui/$(id -u)/com.aiex.scan-button`. Run `./install.sh` again to bring it back.
- Uninstall: `bootout` both `com.aiex.scan-button` and `com.aiex.scan-process` as above, then delete their plists from `~/Library/LaunchAgents`.

Wi-Fi scanning is not supported; [WIFI-SUPPORT-PLAN.md](WIFI-SUPPORT-PLAN.md) has the idea.

The rest of this file describes the Linux setup unless it says otherwise.

Tested on one Linux machine and one Mac with one scanner. The naming and splitting rules are heuristics tuned on English, US-style bills, statements and business cards; expect to adjust them.

The rest of this file is the working notes: how the scanner was set up, how each step works, what was tried and what is still open. Fujitsu's manuals are not included; `docs/README.md` links to them and summarises what is known about the Evernote Edition.

## Hardware

| | |
|---|---|
| Model | Fujitsu ScanSnap iX500 EE |
| USB ID | `04c5:13f3` |
| Connection | USB 3.0 (5000 Mbps), bus 3 (`/dev/bus/usb/003/002` at time of writing) |
| SANE backend | `fujitsu` |
| SANE device name | `fujitsu:ScanSnapiX500EE:17483` |

`lsusb` isn't installed. To list USB devices without it:

```sh
for d in /sys/bus/usb/devices/*; do
  [ -f $d/idVendor ] && echo "$(basename $d) $(cat $d/idVendor):$(cat $d/idProduct) $(cat $d/manufacturer 2>/dev/null) | $(cat $d/product 2>/dev/null)"
done
```

## Setup

1. Install SANE (version 1.4.0-4 from `extra`):

   ```sh
   sudo pacman -S --needed sane
   ```

   Run this in a real terminal window. sudo needs a TTY to prompt for the password, so it fails from Claude Code's `!` prefix (and from any other non-interactive shell) with `sudo: a terminal is required to read the password`.

2. That's it. No extra udev rules are needed. Installing `sane` adds rules that grant the logged-in user read/write access to the device with an ACL:

   ```
   $ getfacl /dev/bus/usb/003/002
   user::rw-
   user:you:rw-
   group::rw-
   group:saned:rw-
   ```

   Before SANE was installed, the device was `root:root crw-rw-r--` and not writable by the user.

3. Verify:

   ```sh
   scanimage -L
   # device `fujitsu:ScanSnapiX500EE:17483' is a FUJITSU ScanSnapiX500EE scanner
   ```

   The integrated webcam also shows up (as `v4l:/dev/video0`). Ignore it.

## Scanner options

From `scanimage -d 'fujitsu:ScanSnapiX500EE:17483' -A`:

| Option | Values | Default |
|---|---|---|
| `--source` | `ADF Front`, `ADF Back`, `ADF Duplex` | `ADF Front` |
| `--mode` | `Lineart`, `Gray`, `Color` | `Lineart` |
| `--resolution` | 50–600 dpi | 600 |

The backend also offers software crop (`--swcrop`) and deskew (`--swdeskew`), plus double-feed detection options.

## Scanning

Test scan (duplex, color, 300 dpi), which writes `page001.png`, `page002.png`, …:

```sh
scanimage -d 'fujitsu:ScanSnapiX500EE:17483' \
  --source 'ADF Duplex' --mode Color --resolution 300 \
  --format=png --batch=page%03d.png
```

Result for one sheet: 2 pages, each 2550×3300 px (Letter at 300 dpi), about 12–13 MB as PNG. The batch ends normally with `sane_start: Document feeder out of documents` when the feeder is empty.

Suggested everyday command (grayscale, crop to the paper, straighten):

```sh
scanimage -d 'fujitsu:ScanSnapiX500EE:17483' \
  --source 'ADF Duplex' --mode Gray --resolution 300 \
  --swcrop=yes --swdeskew=yes \
  --format=png --batch=page%03d.png
```

The software crop and deskew settings haven't been tried yet.

## Scan button → PDF in ~/Notes/Scans

Press the scanner's Scan button and `bin/scan-to-pdf` (Python, stdlib only) runs.

**Scanning and processing are separate, so you can queue scans.** The button only waits for the
scan itself (a few seconds). The pages go into a queue folder (`~/.local/state/scan-queue/<timestamp>/`),
then `scan-process.service` handles the queue in the background, one scan at a time, oldest first, at
low priority. That service doesn't depend on the scanner: closing the lid mid-processing is fine,
and anything left in the queue (e.g. after a logout) is picked up the next time the scanner turns
on. If processing fails, the pages are kept in `~/.local/state/scan-queue/failed/`. Logs:
`journalctl --user -u scan-process -f`.

**Raw scans are kept for a while.** The finished PDFs in `~/Notes/Scans` are the result; nothing
else is needed to keep your documents. As a safety net, the untouched scanner images of the last
three scans stay in `~/.local/state/scan-queue/done/<timestamp>/` (roughly 60 MB per scan, more for
big batches). If a PDF comes out badly cropped, split wrong or upside down, redo it without feeding
the paper again: `scan-to-pdf --from-dir ~/.local/state/scan-queue/done/<timestamp>`. Each new scan
pushes the oldest one out. Keep more or fewer with `SCAN_KEEP_RAW=N` in `~/.config/scan-button.env`;
`SCAN_KEEP_RAW=0` deletes the raw images as soon as the PDFs are saved, and a bad result then means
rescanning.

**Watchdog:** SANE's software crop/deskew once spun forever on one card side (100% CPU, no I/O). If no
new side arrives for 60 s (`SCAN_HANG_SECS`), scanimage is stopped, the sides already scanned are
queued, and a notification tells you which sheet to rescan.

Steps:

1. **Scan:** duplex, 300 dpi color, **raw 14-inch frame** (`--page-height 356`). The scanner's own crop/deskew (`--swcrop/--swdeskew`) is no longer used: it hung three times (100% CPU, no I/O), left cards 1–4° tilted, cut into white cards, and jammed on sheets longer than letter. `SCAN_CROP=scanner` brings it back.
1b. **Own crop** (`autocrop`): the backing is light gray with vertical streaks. The bottom rows of the frame give the backing color per column, and whatever differs (largest channel difference, so pink paper counts) is the sheet. A clean rectangle is straightened by its outline and cropped. Otherwise (crumpled, full-width page) only the backing below the sheet is cut. If the bottom rows aren't backing (sheet as long as the frame), the frame is kept. Text-line leveling then fixes what's left (cards up to 10°, documents up to 3°). Cards also get a light unsharp mask.
   Also in step 1: **Double-feed detection** is on: if the ultrasonic sensor sees two sheets stuck together, the scan stops and you get a critical notification.
2. **Blank sides are dropped.** The test only looks at the inner ~88% × 84% of each side. Real blank card/paper backs measure 0% dark pixels there, but 1–2% on the whole side because of edge shadows (`SCAN_BLANK_PCT`, default 0.05).
3. **Orientation**, per side:
   - documents: tesseract orientation detection at full resolution. If it's unsure (confidence < 2), the script compares how well the page reads upright vs upside down. A confident side of a sheet decides for an unsure one. ocrmypdf's default threshold (14) missed real upside-down pages, which scored 6–14.
   - cards: try all four rotations and keep the one that reads best. Card backs without text (logo, QR) follow the front.
   - cards are then **leveled from their text lines** (`magick -deskew`, applied only for tilts of 0.2–10°), and the exposed corners are filled with the backing color and trimmed. SANE's `--swdeskew` leaves cards 1–3° off: the iX500's backing plate is light gray (~215–235/255), so a white card's edges barely show. That low contrast also makes `--swcrop` occasionally cut into a white card (one card lost the top of its name). Cards are saved at JPEG q92 (`SCAN_CARD_JPEG_QUALITY`).
4. **OCR** per side with tesseract (one thread each, all sides in parallel), giving a one-page PDF with a text layer. Cards are OCR'd at 2× for small print, with a "single block" fallback when no name is found.
5. **Split into documents** (only at sheet boundaries; a document never starts on a back side):
   - a **business-card-sized** sheet (≤ 4.2 × 2.8 in after crop) → its own PDF
   - a **fully blank sheet** → manual separator (always splits)
   - otherwise a new document starts when its score is ≥ 2:
     - **+3** the front says "Page 1 of N" (OCR slips like `10f1` / `70of8` are handled)
     - **+3** the current document is complete (its "N of N" was seen)
     - **+2** the paper size differs from the previous sheet by more than 8% (e.g. a half-page tax bill followed by a letter page)
     - **+2** the front looks like the current document's first page (same template, e.g. a second statement). Thumbnail distance < 0.2 (`SCAN_SIMILAR`); unrelated pages measured 0.24–0.6.
     - **−3** the front continues the current document's numbering ("4 of 8" after "2 of 8")
   - Tested on a 21-page stack of 5 documents (2 statements, an application, 2 bills): split exactly right.
6. **Names** (local heuristics; nothing leaves the machine):
   - documents: `YYYY-MM-DD Sender type.pdf`, e.g. `2026-09-17 Acme Health statement.pdf`. The sender is an explicit "DBA …", else a letterhead line with a company word (trimmed), else the most frequent org email/web domain. The date is a labelled "… Date" (not "Due Date"), else the first date, else today.
   - cards: `Business card - Person (Company).pdf`. The person is a name-like line, preferring one next to a job title; slogans like "ALWAYS CURRENT" are rejected. The company is a company-like line, else the email domain, so OCR typos in domains carry through.
7. **Name already taken** (a scan never overwrites a file):
   - same content as the existing file (same page count, word overlap ≥ 0.8, number overlap ≥ 0.7) → a rescan; saved to `duplicates/` inside the output folder and reported in the notification. Real rescans measured 0.89–0.96 / 0.82–0.90; two months of the same statement 0.86 / 0.56.
   - different content → the name gets what tells them apart: a reference/invoice number or the last 4 digits of an account number (`… - acct …7611.pdf`), else the amount, else `- scanned YYYY-MM-DD`, else `(2)`.
   - `SCAN_DEDUPE=no` goes back to plain `(2)` numbering.
8. **Save + notify:** click the notification to open the PDF, or the folder if the scan produced several files.

Debugging without the scanner: put `p0001.tif/jpg, p0002…` (front/back pairs, including blank backs) in a folder and run `scan-to-pdf --dry-run --from-dir DIR`. It prints the orientation and split decisions and the names.

How it works:

- The iX500 has no USB interrupt endpoint, only `ep_02` (bulk out) and `ep_81` (bulk in). It can't announce a button press, so the computer has to ask for the button state. Fujitsu's own software also asks repeatedly.
- `src/scan-button-watch.c` is a small C program. It keeps one SANE handle open and reads only the `scan` sensor once a second. Measured cost: 0 CPU ticks in 20 s and 6 MB of memory. (The first version spawned `scanimage -A` twice a second and used about 2.7% of a core.) When the button is pressed, it releases the scanner and runs `bin/scan-to-pdf`.
- `udev/99-scansnap.rules` tags the scanner for systemd as `dev-scansnap.device` and asks your user manager to start `scan-button.service` when the scanner appears. The service has `BindsTo=` that device, so it stops when the scanner disappears (lid closed or unplugged). While the scanner is off, nothing runs.
- `systemd/scan-button-login.service` (enabled, runs once at login) starts the watcher if the scanner was already on when you logged in. The udev trigger only fires when the scanner appears during a running session, so after a reboot with the lid open nothing started it.

Install or update with `./install.sh`. It builds the program, installs the user service, and installs the udev rule, which needs sudo once.

`scan-to-pdf` can also be run by hand. Overrides go in `~/.config/scan-button.env`: `SCAN_MODE` (Color/Gray/Lineart), `SCAN_RESOLUTION`, `SCAN_OUTDIR`, `SCAN_BLANK_PCT`, `SCAN_JPEG_QUALITY`, `SCAN_OCR_LANG`, `SCAN_DOUBLE_FEED=no`, `SCAN_SMART_NAMES=no`, `SCAN_SPLIT=no` (one PDF per scan), `SCAN_SIMILAR`, `SCAN_DEDUPE=no`. `SCAN_OVERSCAN=no` (don't start the frame before the sheet), `SCAN_KEEP_RAW=N` (raw frames of the last N scans kept in `~/.local/state/scan-queue/done/`, default 3; reprocess one with `scan-to-pdf --from-dir <that folder>`).

Logs: `journalctl --user -u scan-button -f`. While the service runs, it holds the scanner open, so other apps (simple-scan etc.) will find the device busy. Stop it with `systemctl --user stop scan-button` while you use them. Also stop it before cleaning, because cleaning mode uses the Scan button to turn the rollers.

## Documentation and Evernote Edition

Official manuals (operation guides, safety, datasheet), plus research notes on the Evernote Edition, are in [`docs/`](docs/README.md). Summary: the EE is the same hardware as a standard iX500. It has different firmware and USB ID (`13f3`, versus `132b` for the standard model) and came with Evernote-only software. SANE's fujitsu backend treats it like a normal iX500. PFU's one-way conversion tool is only needed for PFU's Windows/Mac software, so don't convert it for Linux use.

Cleaning: see "Cleaning the Inside of iX500" (p. 526) in `docs/iX500-basic-operation-guide-windows.pdf`. Rollers that need power are turned by pressing Scan with the top open; 7 presses = one full turn.

## Known issues

- **Pages come out upside down.** In the test, the sheet was loaded with the top edge going in last. Load pages face-down with the top edge going in first, or rotate the images afterwards (`magick in.png -rotate 180 out.png`).
- **Large empty border.** Without cropping, the scanner captures a full Letter-size area, so small documents end up with a lot of gray background. Use `--swcrop=yes` or set `--page-width`/`--page-height`.

## Next steps

- [x] Scan button → PDF (watcher + udev-started user service), double-feed stop, blank-side removal.
- [x] Auto-orientation (documents and cards), business cards one per file, splitting a stack into documents.
- [x] Card test session (2026-11-11): `--bgcolor Black` is ignored by the iX500; `--ald` gave no shorter frames and an I/O error on a long sheet; 600 dpi is sharper but 3.3× slower (31.5 s vs 9.5 s for 4 cards); `--emphasis 64` ≈ software sharpening.
- [ ] Tighten the card crop (thin gray shadow margins remain); optional 600 dpi card mode.
- [ ] Splitting: a mixed stack of unrelated one-sheet documents without page numbers still merges.
- [ ] Smaller files: save pages without color as grayscale (currently ~1.7 MB/page at 300 dpi color, JPEG q80).
- [ ] Better names and card company names (e.g. with Claude). That would send OCR text off the machine.
- [ ] Maybe: double-press to append to the previous scan; export cards as vCard (.vcf).

## Card crop bug (fixed 2026-11-16)

A batch of cards scanned on 2026-11-16 came out with one corner cut off (text cut mid-letter) and backing on the opposite corner. Cause: in `autocrop()`, ImageMagick's `-rotate` leaves a negative page offset, and both the `%@` bounding box and `-crop` count from it, so the crop box landed shifted (up to ~75 px at 300 dpi). Fixed with `+repage` after `-rotate`. Reproduced and verified on 16 saved raw card sides (all whole). Changed at the same time: overscan on (untested live), card sides without readable text follow the readable side (mirrored), unreadable portrait cards are laid flat.
