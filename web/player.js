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
  constructor(module, rom) {
    this.m = module;
    const size = (rom.byteLength + 0x7fff) & ~0x7fff;     // align to 32 KiB
    this.romPtr = module._malloc(size);
    this.heap(this.romPtr, size).fill(0).set(rom);
    this.actx = new (window.AudioContext || window.webkitAudioContext)();
    this.e = module._emulator_new_simple(this.romPtr, size, this.actx.sampleRate,
                                         AUDIO_FRAMES, CGB_COLOR_CURVE);
    if (!this.e) throw new Error("Invalid ROM");
    // The emulator reads buttons through this callback (wrapper.c).
    this.joyBuf = module._joypad_new();
    module._emulator_set_default_joypad_callback(this.e, this.joyBuf);

    this.frame = this.heap(module._get_frame_buffer_ptr(this.e),
                           module._get_frame_buffer_size(this.e));
    this.audioBuf = this.heap(module._get_audio_buffer_ptr(this.e),
                              module._get_audio_buffer_capacity(this.e));
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
      if (ev & EVENT_NEW_FRAME) { this.img.data.set(this.frame); this.frames++; }
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
    return storageSet(SAVE_KEY, data);
  }

  loadState() {
    const data = storageGet(SAVE_KEY);
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
    const p = new Player(module, b64ToBytes(window.GAME_ROM_B64));
    window.player = p;                 // used by the automated browser test
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
    if (storageGet(SAVE_KEY)) $("#btn-load").classList.add("has-save");
  } catch (err) {
    console.error(err);
    overlay.querySelector("button").textContent = "Could not start: " + err.message;
  }
})();
