# Wi-Fi support: plan

Status: **idea only.** Nothing here has been tried. The projects below were found in a web search
on 2026-11-23; their code has not been read and none of them has been run.

Today the scanner works over USB only, on Linux and on macOS (see
[MAC-SUPPORT.md](MAC-SUPPORT.md)). SANE's `fujitsu` driver, which everything is built on, talks
to the iX500 over USB and nothing else. The scanner's Wi-Fi uses Fujitsu's own protocol.

So Wi-Fi would be a second way of getting pages into the queue. Everything after the scan
(cropping, OCR, splitting, naming) would be reused unchanged: the worker only needs a folder of
`p0001.tif`, `p0002.tif`, … in `~/.local/state/scan-queue`.

## Existing work to build on

Others have reverse-engineered the protocol.

| Project | What it is | Note |
|---|---|---|
| [scansnack](https://github.com/aijayadams/scansnack) | Python client for the iX500 and iX1500 over Wi-Fi: discovery, pairing, duplex scanning, no ScanSnap Home. Has a protocol write-up per model | Looks like the best starting point |
| [scansnap-docker](https://github.com/Exellent1988/scansnap-docker) | Command-line tool with a written protocol specification (fork of bramheerink/scansnap) | Getting the scanner's pairing key needs the official ScanSnap Home app once |
| [scannerserver](https://github.com/KaiReichart/scannerserver) | Self-hosted server for the iX500 with web UI, Wi-Fi scanning, OCR | Claims button support |
| [ix500ctl](https://github.com/st0ne/ix500ctl) | Sets the scanner's Wi-Fi settings over USB on Linux | Its author says this is not yet verified |

Licenses and how well each is maintained have not been checked.

## Open questions

Each of these could end the idea. They are the first thing to answer, before writing any code.

1. **Getting the scanner onto the network.** Normally done with Fujitsu's setup tool or WPS
   ([Fujitsu's instructions](https://www.pfu.fujitsu.com/imaging/downloads/manual/basic/win/en/topics/connect_wireless_setting.html)).
   Whether the Evernote Edition accepts either is unconfirmed. Is this scanner already joined to
   the Wi-Fi? If not, ix500ctl may be the only route that avoids Fujitsu's software.
2. **The Scan button.** The USB setup polls the button. Can a button press be seen over Wi-Fi?
   scannerserver claims button support, which is a good sign. Without it, Wi-Fi scanning would
   have to be started from the computer, which defeats the point.
3. **Scan options.** The crop relies on a raw frame 356 mm long with overscan, and scanning stops
   on a double feed. Does the Wi-Fi protocol offer these, or something close enough?
4. **Pairing.** Does pairing need ScanSnap Home once (as scansnap-docker says), or can scansnack
   do it alone?
5. **Image format.** What the scanner sends over Wi-Fi (probably JPEG, not raw TIFF) and whether
   the crop and blank-side detection still work on it.

## Steps

1. Read scansnack's iX500 protocol notes and code, and scannerserver's button handling. Answer
   the five questions above on paper. Stop if the button or the network setup can't work.
2. Get the scanner onto the Wi-Fi and make one scan with scansnack as it is.
3. Run those pages through `scan-to-pdf --from-dir` and compare with a USB scan of the same
   sheets.
4. Only then design the integration: most likely a second watcher that waits for the button over
   Wi-Fi and writes the pages into the queue, next to the USB one.

## Size

Unknown until step 1 is done.
