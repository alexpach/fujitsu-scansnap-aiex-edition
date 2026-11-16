# ScanSnap iX500 documentation

Reference material for the Fujitsu (now PFU / Ricoh) ScanSnap iX500. This unit identifies itself
as `ScanSnapiX500EE` (USB `04c5:13f3`), which is the **ScanSnap Evernote Edition**. See the
[Evernote Edition](#evernote-edition) section below.

All files were checked with `file` and `pdfinfo` and are valid PDFs. Collected 2026-11-10.

## Downloaded files

| File | Title / part no. | Source URL | What it covers |
|---|---|---|---|
| `iX500-basic-operation-guide-windows.pdf` | Basic Operation Guide (Windows), P3PC-6912-05ENZ2, 606 pp, 39 MB | https://origin.pfultd.com/downloads/IMAGE/manual/ix500/P3PC-6912-05ENZ2.pdf (linked from the [PFU iX500 manuals page](https://www.pfu.ricoh.com/global/scanners/scansnap/support/manuals/ix500.html)) | The current main manual, written for ScanSnap Manager and covering several models. It includes part names, loading documents, troubleshooting, **cleaning the iX500** (p. ~524), **consumables, part numbers and replacement** (pad and pick roller, p. ~549), and specifications. |
| `iX500-advanced-operation-guide-windows.pdf` | Advanced Operation Guide (Windows), P3PC-4452-17ENZ2, 155 pp | https://origin.pfultd.com/downloads/IMAGE/manual/ix500/P3PC-4452-17ENZ2.pdf | Advanced ScanSnap Manager features (profiles, Quick Menu, scan settings, linked applications). |
| `iX500-operators-guide-2012.pdf` | Operator's Guide, P3PC-4202-01ENZ0 (1st ed., Sept 2012), 480 pp | https://archive.org/download/manualsonline-id-233535ee-89a2-4307-9392-13229ec605d1/233535ee-89a2-4307-9392-13229ec605d1.pdf | The original single-volume iX500 manual: setup, scanning, Wi-Fi, troubleshooting, cleaning, consumables, and specifications. **Third-party mirror** (archive.org, from ManualsOnline), not hosted by PFU. The PDF metadata shows the original Fujitsu FrameMaker file, and the archive.org "Danish" language tag is wrong. It has since been superseded by the Basic and Advanced guides above. |
| `iX500-safety-precautions.pdf` | Safety Precautions, P3PC-4182-07EN, 10 pp | https://origin.pfultd.com/downloads/IMAGE/manual/ix500/Safety_fw_EN.pdf | General safety information for the iX500 and S1300i. |
| `iX500-quick-installation-sheet.pdf` | Quick Installation Sheet, P3PC-4242-07XA, 2 pp | https://origin.pfultd.com/downloads/IMAGE/manual/ix500/P3PC-4242-07XA.pdf | Multilingual unboxing and setup leaflet (removing packaging, connecting, installing software). This is the closest current equivalent to the old "Getting Started" leaflet. |
| `iX500-troubleshooting-installation.pdf` | Troubleshooting (installation), P3PC-4222-04ENZ0, 83 pp | https://origin.pfultd.com/downloads/IMAGE/manual/trouble/p3pc-4222-04enz0.pdf | Fixes for problems installing the bundled Windows and Mac software. Not useful for Linux, but it lists the bundled software. |
| `iX500-datasheet.pdf` | Fujitsu ScanSnap iX500 Datasheet (Fujitsu US, 2016), 4 pp | Wayback copy: https://web.archive.org/web/20240118061714/https://www.fujitsu.com/us/Images/ix500_datasheetv2.pdf (the original at fujitsu.com returned HTTP 429 to curl) | Marketing datasheet with the specification table. |
| `iX500-brochure-2017.pdf` | ScanSnap iX500 Brochure 06 EN01 (2017-04), 2 pp | Wayback copy: https://web.archive.org/web/20240608232132/https://www.fujitsu.com/global/Images/ScanSnap_iX500_Brochure_06_EN01_201704.pdf | General specifications table: ADF, dual CIS, speeds, resolutions, paper sizes, power, and dimensions. |

Every PFU manual for the iX500, in all languages, is listed at
https://www.pfu.ricoh.com/global/scanners/scansnap/support/manuals/ix500.html. The PDFs are hosted under
`https://origin.pfultd.com/downloads/IMAGE/manual/...`.

## Looked for but not saved

- **Separate "Getting Started" PDF for the iX500 (P3PC-4162-xxEN).** I found no official PDF online.
  Third-party viewers exist at https://www.manualslib.com/manual/559136/Fujitsu-Scansnap-Ix500.html and
  https://www.manualowl.com/m/Konica%20Minolta/Fujitsu-ScanSnap-iX500/Manual/494437?page=1. They are HTML
  viewers, not PDFs. The Quick Installation Sheet above covers the same ground.
- **A dedicated troubleshooting, cleaning or consumables guide.** PFU does not publish one as a separate PDF.
  That material is in the Basic Operation Guide and the 2012 Operator's Guide above. PFU also has online
  pages for it:
  - Consumables part numbers: https://www.pfu.ricoh.com/imaging/downloads/manual/ssca-ios/English/ssca/topic/consumable_specification.html
  - Maintenance: https://www.pfu.fujitsu.com/imaging/downloads/manual/ss_webhelp/en/help/webhelp/topic/ix500_maintenance_top.html
  - The consumable is the **Roller Set PA03656-0001** (pad/brake roller and pick roller). The suggested
    replacement interval is 200,000 sheets or one year.
- **Evernote Edition-specific manual or guide.** I found none on PFU's sites or in Wayback. As far as I
  could tell, ScanSnap Manager Evernote Edition shipped with built-in help only, and the EE product pages
  link no PDFs. The only official EE documentation is the web page
  https://www.pfu.ricoh.com/global/scanners/scansnap/evernote/index.html and its functional-differences page.
- **Basic and Advanced Operation Guide (Mac OS).** These are available (P3PC-6932-05ENZ2, 34 MB, and
  P3PC-4862-16ENZ2) at the PFU manuals page above. I left them out because they duplicate the Windows
  content and are not relevant on Linux.
- **ScanSnap Wireless Setup Tool Help.** Available as P2WW-4510-03ENZ2 (Windows) and P2ZZ-0440-03ENZ2 (Mac).
  I skipped it because it is not relevant to SANE over USB.
- **iFixit copy of the Operator's Guide, revision 03 (P3PC-4202-03ENZ0).** Available at
  https://www.ifixit.com/Document/nexDeWcg11WeYIBj/ScanSnap-iX500-Operators-Guide.pdf. I did not download it;
  the archive.org copy of revision 01 is saved instead.
- **Japanese Operator's Guide on PFU's site.** Available at
  https://www.pfu.ricoh.com/global/scanners/scansnap/downloads/jp/jscan/manual/p3pc-4202-04z0.pdf.
  I skipped it because it is not in English.

## Evernote Edition

**Identity.** The ScanSnap Evernote Edition (model **FI-IX500EN**) was announced and sold through the Evernote Market
from 27 Sept 2013 (JP price ¥49,800), with shipping from 25 Oct 2013. The standard model is **FI-IX500A**.
- Sources: https://internet.watch.impress.co.jp/docs/news/617107.html and https://www.pfu.ricoh.com/imaging/evernote/

**Hardware.** The hardware is the same as the iX500. Impress Watch says the hardware specifications match the
standard iX500. The differences are cosmetic: a matte grey case with an Evernote logo, a dot pattern on the ADF
cover, and a **green** scan-button LED instead of ScanSnap blue. The consumables (PA03656-0001) and the
manuals listed above therefore apply.
- Sources: Impress Watch (above) and https://www.documentsnap.com/?p=4008

**What differs is the firmware and USB identity.** The EE runs different firmware and enumerates with a different USB ID:

| | USB ID | SANE model string |
|---|---|---|
| Standard iX500 | `04c5:132b` | ScanSnap iX500 |
| Evernote Edition (this unit) | `04c5:13f3` | ScanSnap iX500EE |

Both IDs are listed in sane-backends' `fujitsu.conf.in` and `doc/descriptions/fujitsu.desc`, so the `132b`
value for the standard model is confirmed. PFU's official conversion tool "updates the firmware in ScanSnap
Evernote Edition thus changing the scanner to ScanSnap iX500 … After the update process is complete, the scanner
is recognized as ScanSnap iX500".
- Sources: Wayback copy of PFU's conversion page,
  https://web.archive.org/web/20210619163124/http://scansnap.fujitsu.com/global/evernote/
- *Inference, not confirmed by a primary source:* after conversion the unit probably reports `04c5:132b`.

**Bundled software.** The EE shipped with **ScanSnap Manager Evernote Edition**, a separate driver that
auto-classifies scans as documents, business cards, receipts or photos and files them into Evernote notebooks
with tags. It also came with a one-year Evernote Premium coupon. It did **not** include the standard iX500
bundle: Adobe Acrobat Standard, CardMinder, ABBYY FineReader OCR, or ScanSnap Organizer. The EE driver only
targets Evernote. The EE's mobile app was Evernote's Scannable (iOS), not ScanSnap Connect.
- Sources: Impress Watch (above) and https://www.pfu.ricoh.com/global/scanners/scansnap/evernote/function.html

**Compatibility with standard ScanSnap Manager or ScanSnap Home.**
- Standard ScanSnap Manager does not recognise an unconverted EE. This is per a DocumentSnap test and reader
  comments, consistent with the different USB ID. Treat it as anecdotal; I found no official statement.
- PFU's own path to ScanSnap Home is to **convert the EE first**, then install ScanSnap Home and choose "iX500".
- *Uncertain:* I found no statement on whether ScanSnap Home detects an unconverted EE (`13f3`). PFU's
  documentation implies it does not.
- Sources: https://www.documentsnap.com/?p=4008, https://www.documentsnap.com/?p=6000 and
  https://www.pfu.ricoh.com/global/scanners/scansnap/evernote/index.html

**Conversion to a standard iX500.**
- PFU offered "ScanSnap Evernote Edition Convert Tool" for Windows (`SSEEConvertTool.zip`) and Mac
  (`SSEEConvertTool.dmg`). It runs over USB and rewrites the firmware.
- The conversion is **one-way**: "Once you have changed ScanSnap Evernote Edition to ScanSnap iX500, you cannot
  change it back." EE-only features are lost: choosing the document type at scan time, auto-tagging, and
  Scannable support.
- PFU says distribution of the tool and of ScanSnap Manager EE **ended on 31 March 2023**.
- *Unverified:* the original tool URLs (`http://origin.pfultd.com/downloads/ss/sshinst/w-tool/SSEEConvertTool.zip`
  and `.../m-tool/SSEEConvertTool.dmg`) still answered HTTP 200 to a HEAD request on 2026-11-10, and Wayback
  has copies. I did not download or test them. They need Windows or macOS with the EE driver installed.
- Sources: https://www.pfu.ricoh.com/global/scanners/scansnap/evernote/index.html and the Wayback copy above.

**Support status.**
- The EE is discontinued, and its driver and conversion tool were withdrawn on 2023-03-31.
- Standard ScanSnap Manager was itself withdrawn on 2024-11-07, leaving ScanSnap Home as PFU's only driver.
- The standard iX500 still receives some support. For example, PFU announced a mandatory ScanSnap Cloud firmware
  update for iX500, iX100, iX1300, iX1500 and iX1600 to be applied by 2026-03-25. I did not check whether this
  is relevant or applicable to an unconverted EE; probably not.
- Sources: https://www.pfu.ricoh.com/global/scanners/scansnap/news/info.html and https://www.pfu.ricoh.com/imaging/evernote/

**SANE / Linux.**
- sane-backends' fujitsu backend lists `usb 0x04c5 0x13f3` as "ScanSnap iX500EE" in `fujitsu.conf.in`.
- `fujitsu.desc` marks it `:status :untested`, while the standard iX500 (`132b`) is `:good`.
- In `backend/fujitsu.c`, model quirks are selected with `strstr(model_name, "iX500")`, so the EE gets the same
  iX500 handling as the standard model: software-generated grayscale and lineart, `need_diag_preread`,
  `hopper_before_op`, and no halftone. Because the hardware is the same, the EE should behave like an iX500
  under SANE. In practice it works for this user.
- Wi-Fi scanning is not supported by SANE for either model.
- Sources: https://gitlab.com/sane-project/backends/-/blob/master/backend/fujitsu.conf.in,
  https://gitlab.com/sane-project/backends/-/blob/master/doc/descriptions/fujitsu.desc and
  https://gitlab.com/sane-project/backends/-/blob/master/backend/fujitsu.c
- *Uncertain:* the exact sane-backends release that added `13f3`. Git history shows it was present by the
  2015–2017 fujitsu backend versions.

**Bottom line for Linux use.** No conversion is needed. SANE drives the EE as an iX500, and conversion would only
matter for using PFU's Windows or Mac ScanSnap Home software. Conversion is irreversible, and the official tool
has been officially withdrawn.
