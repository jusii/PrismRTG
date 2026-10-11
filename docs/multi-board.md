# Two boards at once (design, October 2026)

Goal: PrismRTG driving two graphics boards in one Amiga at the same time - each with its own
screens on its own monitor (step 1) - and later one screen stretched across both monitors
(step 2). Until 1.1 beta 6 PrismD drove one board: with a ZZ9000 and a GBAPII++ fitted it took
the first its drivers found and left the other alone.

**State (October 2026):** steps 1-5 below are done and pass in WinUAE-Prism's `dual-060.uae`
(every drawing check on both boards, each board showing its own front screen, the pointer
following the front screen, dragging on either board); not yet tried on real hardware with
two cards in one machine.

Developed and tested on the emulated boards in WinUAE-Prism (`dual-060.uae`: a 68060 with a
ZZ9000 and a GBAPII++ on a second monitor), then on real hardware.

## What is per board today

Everything below is one global now and becomes one per board, collected in `struct PrismCard`:

| State | Where | Notes |
|---|---|---|
| `board` (the driver's `PrismBoard`: hooks, VRAM, flags, limits) | prismd.c, used in 9 files (~170 places) | the drawing hot paths read it on every call |
| mode table (`modes`, `nmodes`) and the display-database monitor (`prismMonitor`) | prismd.c | each board gets its own monitor, so its modes have their own ModeIDs |
| VRAM allocator (`used`, `nused`) | prismd.c | per board: a bitmap lives in one board's memory |
| what the board shows: `shown`, `shownVP`, `cardMode`, `shownPbm`, `showStamp`, `rtgOn`, palette shadow | prismd.c | each board shows its own frontmost screen, or the native display passes through |
| test picture (`testPbm` ...) | prismd.c | PrismPrefs' Test button targets one board |
| pointer state | pointer.c | only the board showing the front screen shows the pointer |
| compositor (`frame`, row state) | present.c | dragging and PIPs, per board |
| `textBad` | selftest.c | the text self-test result, per board |
| driver process | driver_loader.c | one module per board (two different drivers at first) |

Stays shared: the drawing lock (one lock for all boards keeps the lock order as it is), the
preferences, the list of PrismRTG screens, the patches, cybergraphics.library and
Picasso96API.library.

## How things find their board

* **A bitmap** records its board: `PBitMap.card`. Drawing into it uses that board's hooks and
  limits; its VRAM comes from that board's allocator. A bitmap made with a friend lands on the
  friend's board. Bitmaps not yet in VRAM (fast RAM) still belong to a board, the one they would
  be shown on.
* **A screen** belongs to the board whose monitor its ModeID is on. Board 0 keeps the
  `PRISM:` monitor and ModeIDs `$7A00xxxx` (nothing changes for existing setups and prefs);
  board 1 registers `PRISM2:` with `$7A01xxxx`. ScreenMode and every program pick a board by
  picking a mode, as with any multi-monitor Amiga system.
* **What a board shows** is the frontmost screen on that board's monitor: on screen depth
  changes PrismD looks at each board separately. A board whose screens are all closed or behind
  native screens switches its monitor back to the Amiga (as one board does now).
* **The pointer** follows Intuition: it is on the front screen. The board showing the front
  screen shows the pointer; the other board hides its own.

## Speed

The hot paths read `board.fillRect` etc. from a global today. With two boards they read them
through the bitmap's board: one more load per call. That must not cost measurable speed on a
68030 - each step is benchmarked on the cycle-exact 030 box against the step before (three code
alignments averaged, as in the beta 5 speed work), and on the A4000 when it is available.

## Steps

1. **`struct PrismCard`, one board** - move the per-board state into `cards[0]`, with the old
   names pointing there. No change in behaviour or speed; all seven emulated boxes must pass.
2. **Bitmaps and screens carry their board** - `PBitMap.card`, `PScreen.card`; drawing, VRAM
   allocation and display go through them. Still one board loaded. Speed checked.
   In detail:
   * `PBitMap` gets `card` (its `struct PrismCard`) and `bd` (that card's `PrismBoard`, kept
     beside it so the drawing fast paths load the board in one step). Set where a bitmap is
     made: a screen's from its mode, a friend's from the friend, anything else card 0.
   * The fast paths in `prismint.h` (`HW_OK`, `hw_fits`, `pbm_fill`, `pbm_copy`, `pbm_line`,
     `pbm_expand`, `BOARD_CAN`) and `pbm_fits` take the board from the bitmap. A copy between
     bitmaps of two boards goes through the CPU.
   * The VRAM allocator takes the card: `vram_alloc`/`vram_free`/`vram_get_size`/
     `vram_free_bytes` work on that card's block list (`cst[n].blk`).
   * `ModeRec` gets `card`; `PScreen` takes its card from its mode.
   * The compositor, pointer, Test picture and text self-test use the card of the screen they
     work on (still always card 0 here; per card in step 4).
   * The global `board` stays as card 0's storage until step 3.
3. **Load two boards** - Auto loads every board its drivers find (or `BOARD=` / a new `BOARD2=`
   says which); the second registers `PRISM2:`. Screens on board 1 open and draw; all checks
   pass on both boards in `dual-060.uae`.
4. **Show both** - each board shows its own front screen and switches on its own; pointer on the
   front screen's board; compositor and Test button per board.
5. **PrismPrefs and docs** - the second board's modes in PrismPrefs, PrismCheck reports both
   boards, README.

Then step 2 of the goal (one screen across both monitors) is its own design.

## Not in this first version

Two boards of the same kind (two ZZ9000s): the drivers keep per-board state in statics, so that
needs driver changes too. Moving a screen from one board to the other while it is open.
