/*
 * PARIS DISPARU web player. (c) Jiwon.
 * Runs the Game Boy Color ROM in the binjgb emulator (MIT, Ben Smith; see
 * vendor/binjgb/LICENSE). The emulator loop and audio code follow binjgb's
 * docs/simple.js.
 *
 * The ROM and the emulator's wasm are embedded as base64 by build.py
 * (game-data.js), so the page also works when opened from a local file.
 */
"use strict";

const SCREEN_W = 160, SCREEN_H = 144;
const CPU_TICKS_PER_SEC = 4194304;
const MAX_UPDATE_SEC = 5 / 60;          // never run more than 5 frames per step
const AUDIO_FRAMES = 4096, AUDIO_LATENCY_SEC = 0.1;
const EVENT_NEW_FRAME = 1, EVENT_AUDIO_BUFFER_FULL = 2, EVENT_UNTIL_TICKS = 4;
const CGB_COLOR_CURVE = 2;              // Gambatte-like LCD colours
const SAVE_KEY = "paris-disparu-state-1";
const EPISODE_KEY = "paris-disparu-episode";
// Episodes in this page: Episode 1, and Episode 2 when build.py embedded it.
// Header and story text of each, shown when it is playing.
const EPISODES = [
  { rom: "GAME_ROM_B64", key: SAVE_KEY, h1: null },
  { rom: "GAME_ROM2_B64", key: "paris-disparu-2-state-1", h1: "PARIS DISPARU 2",
    sub: "The last night · Les Halles, 1969",
    story: "Paris, 27 February 1969: the last night of the Les Halles market. Fernand's brass " +
           "porter's plate has been stolen. Ride the métro, play, talk and look, and get it back before dawn." },
].filter((ep) => window[ep.rom]);
const BUTTONS = ["up", "down", "left", "right", "a", "b", "start", "select"];

// Keyboard: event.code is the physical key, so it also works with a Korean
// (or any other) layout or IME switched on.
const KEYMAP = {
  ArrowUp: "up", ArrowDown: "down", ArrowLeft: "left", ArrowRight: "right",
  KeyW: "up", KeyS: "down", KeyA: "left", KeyD: "right",
  KeyX: "a", KeyK: "a", Space: "a",
  KeyZ: "b", KeyJ: "b",
  Enter: "start", NumpadEnter: "start",
  ShiftRight: "select", Backspace: "select",
};
// Standard gamepad mapping (Xbox / PlayStation / Switch Pro in a browser).
const PADMAP = { 0: "a", 1: "b", 2: "b", 3: "a", 9: "start", 8: "select",
                 12: "up", 13: "down", 14: "left", 15: "right" };

const $ = (s) => document.querySelector(s);
const b64ToBytes = (s) => Uint8Array.from(atob(s), (c) => c.charCodeAt(0));
const bytesToB64 = (u8) => {
  let s = "";
  for (let i = 0; i < u8.length; i += 0x8000)
    s += String.fromCharCode.apply(null, u8.subarray(i, i + 0x8000));
  return btoa(s);
};
function storageGet(k) { try { return localStorage.getItem(k); } catch (e) { return null; } }
function storageSet(k, v) { try { localStorage.setItem(k, v); return true; } catch (e) { return false; } }

class Player {
  constructor(module, rom, saveKey) {
    this.m = module;
    this.actx = new (window.AudioContext || window.webkitAudioContext)();
    this.joyBuf = module._joypad_new();
    this.load(rom, saveKey);
    const canvas = $("#screen");
    this.ctx = canvas.getContext("2d");
    this.img = this.ctx.createImageData(SCREEN_W, SCREEN_H);

    this.src = { key: new Set(), touch: new Set(), pad: new Set() };
    this.held = Object.fromEntries(BUTTONS.map((b) => [b, false]));
    // A quick tap can press and release between two frames; keep every press
    // down for a few frames so the game always sees it.
    this.latch = Object.fromEntries(BUTTONS.map((b) => [b, 0]));
    this.volume = 0.5;
    this.muted = false;
    this.running = false;
    this.lastSec = 0;
    this.leftover = 0;
    this.audioStart = 0;
    this.frames = 0;                    // counted for the automated test
  }

  heap(ptr, size) { return new Uint8Array(this.m.HEAP8.buffer, ptr, size); }

  // Put a ROM in the emulator (a new one replaces the old: Episode 1 -> 2).
  // Its cartridge RAM (the game's own save) is restored from localStorage.
  load(rom, saveKey) {
    const m = this.m;
    if (this.e) {
      this.saveExtRam();
      m._emulator_delete(this.e);
      m._free(this.romPtr);
    }
    this.saveKey = saveKey;                              // F6 / F9 state
    this.sramKey = saveKey.replace("-state-", "-sram-"); // in-game save
    const size = (rom.byteLength + 0x7fff) & ~0x7fff;    // align to 32 KiB
    this.romPtr = m._malloc(size);
    this.heap(this.romPtr, size).fill(0).set(rom);
    this.e = m._emulator_new_simple(this.romPtr, size, this.actx.sampleRate,
                                    AUDIO_FRAMES, CGB_COLOR_CURVE);
    if (!this.e) throw new Error("Invalid ROM");
    // The emulator reads buttons through this callback (wrapper.c).
    m._emulator_set_default_joypad_callback(this.e, this.joyBuf);
    this.frame = this.heap(m._get_frame_buffer_ptr(this.e), m._get_frame_buffer_size(this.e));
    this.audioBuf = this.heap(m._get_audio_buffer_ptr(this.e), m._get_audio_buffer_capacity(this.e));
    this.held = Object.fromEntries(BUTTONS.map((b) => [b, false]));
    this.leftover = 0;
    this.loadExtRam();
  }

  // Cartridge RAM <-> localStorage, so "Continue" works after the page is
  // closed, as on a real cartridge with a battery.
  withExtRam(fn) {
    const fd = this.m._ext_ram_file_data_new(this.e);
    const buf = this.heap(this.m._get_file_data_ptr(fd), this.m._get_file_data_size(fd));
    const out = fn(fd, buf);
    this.m._file_data_delete(fd);
    return out;
  }

  loadExtRam() {
    const data = storageGet(this.sramKey);
    if (!data) return;
    const bytes = b64ToBytes(data);
    this.withExtRam((fd, buf) => {
      if (buf.length !== bytes.length) return;
      buf.set(bytes);
      this.m._emulator_read_ext_ram(this.e, fd);
    });
  }

  saveExtRam() {
    return this.withExtRam((fd, buf) => {
      this.m._emulator_write_ext_ram(this.e, fd);
      storageSet(this.sramKey, bytesToB64(buf));
      return buf.slice();
    });
  }

  // Called a few times a second: keep the in-game save, and see whether the
  // game asks for Episode 2 (Episode 1 writes "EP2!" at cartridge RAM 0x80).
  checkExtRam() {
    if (!this.m._emulator_was_ext_ram_updated(this.e)) return;
    const ram = this.saveExtRam();
    if (this.onEpisode2 && ram[0x80] === 0x45 && ram[0x81] === 0x50 && ram[0x82] === 0x32 && ram[0x83] === 0x21) {
      this.withExtRam((fd, buf) => {    // done: clear it, not again next visit
        buf.set(ram);
        buf[0x80] = 0;
        this.m._emulator_read_ext_ram(this.e, fd);
      });
      this.saveExtRam();
      setTimeout(this.onEpisode2);      // after this emulator step

    }
  }

  get ticks() { return this.m._emulator_get_ticks_f64(this.e); }

  start() {
    if (this.running) return;
    this.running = true;
    this.lastSec = 0;
    this.leftover = 0;
    this.audioStart = 0;
    this.actx.resume();
    this.raf = requestAnimationFrame((t) => this.step(t));
  }

  stop() {
    this.running = false;
    cancelAnimationFrame(this.raf);
    this.actx.suspend();
  }

  // Merge keyboard, touch and gamepad; tell the emulator only what changed.
  applyInput() {
    for (const b of BUTTONS) {
      let down = this.src.key.has(b) || this.src.touch.has(b) || this.src.pad.has(b);
      if (this.latch[b] > 0) { this.latch[b]--; down = true; }
      if (down !== this.held[b]) {
        this.held[b] = down;
        const fn = { a: "_set_joyp_A", b: "_set_joyp_B" }[b] || "_set_joyp_" + b;
        this.m[fn](this.e, down ? 1 : 0);
      }
    }
  }

  pollGamepad() {
    const pads = navigator.getGamepads ? navigator.getGamepads() : [];
    const s = this.src.pad;
    s.clear();
    for (const p of pads) {
      if (!p) continue;
      p.buttons.forEach((btn, i) => { if (btn.pressed && PADMAP[i]) s.add(PADMAP[i]); });
      const [x = 0, y = 0] = p.axes;
      if (x < -0.5) s.add("left"); else if (x > 0.5) s.add("right");
      if (y < -0.5) s.add("up"); else if (y > 0.5) s.add("down");
    }
  }

  step(ms) {
    if (!this.running) return;
    this.raf = requestAnimationFrame((t) => this.step(t));
    this.pollGamepad();
    this.applyInput();
    const sec = ms / 1000;
    const dt = Math.min(Math.max(sec - (this.lastSec || sec), 0), MAX_UPDATE_SEC);
    this.lastSec = sec;
    const until = this.ticks + dt * CPU_TICKS_PER_SEC - this.leftover;
    for (;;) {
      const ev = this.m._emulator_run_until_f64(this.e, until);
      if (ev & EVENT_NEW_FRAME) {
        this.img.data.set(this.frame);
        if ((++this.frames & 15) === 0) this.checkExtRam();
      }
      if (ev & EVENT_AUDIO_BUFFER_FULL) this.pushAudio();
      if (ev & EVENT_UNTIL_TICKS) break;
    }
    this.leftover = (this.ticks - until) | 0;
    this.ctx.putImageData(this.img, 0, 0);
  }

  pushAudio() {
    const ctx = this.actx;
    if (ctx.state !== "running") return;
    const now = ctx.currentTime;
    if (this.audioStart < now) this.audioStart = now + AUDIO_LATENCY_SEC;
    const buf = ctx.createBuffer(2, AUDIO_FRAMES, ctx.sampleRate);
    const l = buf.getChannelData(0), r = buf.getChannelData(1);
    const vol = this.muted ? 0 : this.volume / 255;
    for (let i = 0; i < AUDIO_FRAMES; i++) {
      l[i] = this.audioBuf[2 * i] * vol;
      r[i] = this.audioBuf[2 * i + 1] * vol;
    }
    const node = ctx.createBufferSource();
    node.buffer = buf;
    node.connect(ctx.destination);
    node.start(this.audioStart);
    this.audioStart += AUDIO_FRAMES / ctx.sampleRate;
  }

  // Save states (the cartridge itself has no save memory).
  withStateFile(fn) {
    const fd = this.m._state_file_data_new(this.e);
    const buf = this.heap(this.m._get_file_data_ptr(fd), this.m._get_file_data_size(fd));
    const out = fn(fd, buf);
    this.m._file_data_delete(fd);
    return out;
  }

  saveState() {
    const data = this.withStateFile((fd, buf) => {
      this.m._emulator_write_state(this.e, fd);
      return bytesToB64(buf);
    });
    return storageSet(this.saveKey, data);
  }

  loadState() {
    const data = storageGet(this.saveKey);
    if (!data) return false;
    const bytes = b64ToBytes(data);
    return this.withStateFile((fd, buf) => {
      if (buf.length !== bytes.length) return false;
      buf.set(bytes);
      return this.m._emulator_read_state(this.e, fd) === 0;
    });
  }
}

function toast(msg) {
  const t = $("#toast");
  t.textContent = msg;
  t.classList.add("show");
  clearTimeout(toast.id);
  toast.id = setTimeout(() => t.classList.remove("show"), 1600);
}

function bindKeyboard(p) {
  addEventListener("keydown", (e) => {
    if (e.code === "F6" || e.code === "F9") {
      e.preventDefault();
      if (!e.repeat) (e.code === "F6" ? saveAction : loadAction)(p);
      return;
    }
    const b = KEYMAP[e.code];
    if (!b || e.ctrlKey || e.metaKey || e.altKey) return;
    e.preventDefault();                // keep arrows/space from scrolling the page
    if (!p.src.key.has(b)) p.latch[b] = 3;
    p.src.key.add(b);
  });
  addEventListener("keyup", (e) => {
    const b = KEYMAP[e.code];
    if (b) { e.preventDefault(); p.src.key.delete(b); }
  });
  addEventListener("blur", () => p.src.key.clear());   // no stuck keys
}

// On-screen pad. Pointer events cover touch, pen and mouse; each pointer
// remembers what it presses so two thumbs (D-pad + A) work together.
function bindTouch(p) {
  const active = new Map();          // pointerId -> Set of buttons
  const refresh = () => {
    p.src.touch.clear();
    for (const s of active.values()) for (const b of s) p.src.touch.add(b);
    document.querySelectorAll("[data-btn]").forEach((el) =>
      el.classList.toggle("down", p.src.touch.has(el.dataset.btn)));
    $("#dpad").dataset.dir = ["up", "down", "left", "right"].filter((d) => p.src.touch.has(d)).join(" ");
  };
  const dpad = $("#dpad");
  const dpadDirs = (e) => {
    const r = dpad.getBoundingClientRect();
    const x = (e.clientX - r.left) / r.width * 2 - 1;
    const y = (e.clientY - r.top) / r.height * 2 - 1;
    const s = new Set();
    if (Math.hypot(x, y) < 0.2) return s;            // dead zone in the centre
    const ang = Math.atan2(y, x) * 180 / Math.PI;    // 8 sectors, diagonals allowed
    if (ang > -157.5 && ang < -22.5) s.add("up");
    if (ang > 22.5 && ang < 157.5) s.add("down");
    if (ang > 112.5 || ang < -112.5) s.add("left");
    if (ang > -67.5 && ang < 67.5) s.add("right");
    return s;
  };
  const press = (e) => {
    const el = e.target.closest("[data-btn], #dpad");
    if (!el) return;
    e.preventDefault();
    el.setPointerCapture?.(e.pointerId);
    const btns = el === dpad ? dpadDirs(e) : new Set([el.dataset.btn]);
    for (const b of btns) p.latch[b] = 3;
    active.set(e.pointerId, btns);
    refresh();
    if (navigator.vibrate && e.pointerType === "touch") navigator.vibrate(8);
  };
  const move = (e) => {
    if (!active.has(e.pointerId) || e.target.closest("#dpad") !== dpad) return;
    active.set(e.pointerId, dpadDirs(e));
    refresh();
  };
  const release = (e) => { if (active.delete(e.pointerId)) refresh(); };
  const pad = $("#pad");
  pad.addEventListener("pointerdown", press);
  pad.addEventListener("pointermove", move);
  pad.addEventListener("pointerup", release);
  pad.addEventListener("pointercancel", release);
  pad.addEventListener("contextmenu", (e) => e.preventDefault());
}

function saveAction(p) { toast(p.saveState() ? "Game saved" : "Could not save (storage blocked)"); }
function loadAction(p) { toast(p.loadState() ? "Game loaded" : "No saved game yet"); }

function bindUi(p) {
  $("#btn-save").onclick = () => saveAction(p);
  $("#btn-load").onclick = () => loadAction(p);
  const mute = $("#btn-mute");
  mute.onclick = () => {
    p.muted = !p.muted;
    mute.textContent = p.muted ? "Sound: off" : "Sound: on";
    mute.setAttribute("aria-pressed", String(p.muted));
  };
  $("#btn-full").onclick = () => {
    const el = $("#console");
    if (document.fullscreenElement) document.exitFullscreen();
    else (el.requestFullscreen || el.webkitRequestFullscreen)?.call(el);
  };
  $("#btn-help").onclick = () => $("#help").toggleAttribute("open");
  // Pause when the tab is hidden, so the game waits for the player.
  document.addEventListener("visibilitychange", () => {
    if (!p.started) return;
    if (document.hidden) p.stop(); else p.start();
  });
}

(async function main() {
  const overlay = $("#overlay");
  try {
    // Hand the embedded wasm to the module instead of letting it fetch() the
    // file, which browsers refuse for file:// pages.
    const wasm = b64ToBytes(window.BINJGB_WASM_B64);
    const module = await Binjgb({
      instantiateWasm(imports, done) {
        WebAssembly.instantiate(wasm, imports).then((r) => done(r.instance));
        return {};
      },
    });
    const head = $("header h1"), sub = $("header p"), story = $("aside .card:nth-child(2) p");
    EPISODES[0].h1 = head.textContent;
    EPISODES[0].sub = sub.textContent;
    EPISODES[0].story = story.textContent;
    let ep = EPISODES.length > 1 && storageGet(EPISODE_KEY) === "2" ? 1 : 0;
    const p = new Player(module, b64ToBytes(window[EPISODES[ep].rom]), EPISODES[ep].key);
    window.player = p;                 // used by the automated browser test
    const epBtn = $("#btn-ep");
    const show = () => {
      const e = EPISODES[ep];
      head.textContent = e.h1;
      sub.textContent = e.sub;
      story.textContent = e.story;
      document.title = e.h1.replace(/(^|\s)(\S)(\S*)/g, (m, s, a, b) => s + a + b.toLowerCase());
      epBtn.hidden = EPISODES.length < 2;
      epBtn.textContent = ep ? "◀ Episode 1" : "Episode 2 ▶";
      $("#btn-load").classList.toggle("has-save", !!storageGet(p.saveKey));
      // Episode 1's end menu, "Go to Episode 2", lands here.
      p.onEpisode2 = !ep && EPISODES.length > 1 ? () => switchTo(1) : null;
    };
    const switchTo = (i) => {
      ep = i;
      storageSet(EPISODE_KEY, String(i + 1));
      p.load(b64ToBytes(window[EPISODES[i].rom]), EPISODES[i].key);
      show();
      toast(EPISODES[i].h1);
    };
    epBtn.onclick = () => switchTo(ep ? 0 : 1);
    addEventListener("pagehide", () => p.saveExtRam());
    show();
    bindKeyboard(p);
    bindTouch(p);
    bindUi(p);
    overlay.querySelector("button").disabled = false;
    overlay.querySelector("button").textContent = "▶  Play";
    const begin = () => {
      if (p.started) return;
      overlay.hidden = true;
      p.started = true;
      p.start();
      $("#screen").focus();
    };
    overlay.addEventListener("click", begin);
    addEventListener("keydown", (e) => {
      if (p.started || (e.code !== "Enter" && e.code !== "Space")) return;
      e.preventDefault();
      e.stopImmediatePropagation();    // this key press only starts the player
      begin();
    }, true);
  } catch (err) {
    console.error(err);
    overlay.querySelector("button").textContent = "Could not start: " + err.message;
  }
})();
