// Browser test for the web player (Playwright + Chromium, headless).
//   python3 web/build.py && node web/test_web.js [shot_dir]
// Checks: the page loads from a web server and from file://, the emulator
// draws frames, keyboard / on-screen touch buttons reach the game, save and
// load work, and no errors are logged.
"use strict";
const { chromium, devices } = require("playwright");
const http = require("http");
const fs = require("fs");
const path = require("path");

const DIR = path.resolve(__dirname, "..", "build", "web");
const SHOTS = process.argv[2];
let failed = 0;
function check(cond, msg) {
  console.log((cond ? "  ok   " : "  FAIL ") + msg);
  if (!cond) failed++;
}

function serve() {
  const types = { ".html": "text/html", ".js": "text/javascript", ".wasm": "application/wasm" };
  const srv = http.createServer((req, res) => {
    const f = path.join(DIR, decodeURIComponent(req.url.split("?")[0]).replace(/\/$/, "/index.html"));
    if (!f.startsWith(DIR) || !fs.existsSync(f)) { res.writeHead(404); return res.end(); }
    res.writeHead(200, { "Content-Type": types[path.extname(f)] || "text/plain" });
    fs.createReadStream(f).pipe(res);
  });
  return new Promise((ok) => srv.listen(0, "127.0.0.1", () => ok(srv)));
}

const pixels = (page) => page.evaluate(() =>
  Array.from(document.querySelector("#screen").getContext("2d").getImageData(0, 0, 160, 144).data));
const diff = (a, b) => { let n = 0; for (let i = 0; i < a.length; i += 4) if (a[i] !== b[i] || a[i + 1] !== b[i + 1] || a[i + 2] !== b[i + 2]) n++; return n; };
const colours = (p) => { const s = new Set(); for (let i = 0; i < p.length; i += 4) s.add((p[i] << 16) | (p[i + 1] << 8) | p[i + 2]); return s.size; };
const frames = (page) => page.evaluate(() => window.player ? window.player.frames : -1);
const wait = (page, ms) => page.waitForTimeout(ms);
async function shot(page, name) {
  if (SHOTS) { fs.mkdirSync(SHOTS, { recursive: true }); await page.screenshot({ path: path.join(SHOTS, name + ".png") }); }
}
async function tapKey(page, code, ms = 120) {
  await page.keyboard.down(code); await wait(page, ms); await page.keyboard.up(code);
}

async function open(browser, url, ctxOpts = {}) {
  const ctx = await browser.newContext(ctxOpts);
  const page = await ctx.newPage();
  const errors = [];
  page.on("pageerror", (e) => errors.push(String(e)));
  page.on("console", (m) => { if (m.type() === "error") errors.push(m.text()); });
  await page.goto(url);
  await page.waitForFunction(() => !document.querySelector("#overlay button").disabled, null, { timeout: 15000 });
  return { ctx, page, errors };
}

(async () => {
  const srv = await serve();
  const base = `http://127.0.0.1:${srv.address().port}/index.html`;
  const browser = await chromium.launch({ args: ["--autoplay-policy=no-user-gesture-required"] });

  console.log("desktop, keyboard (served over http)");
  {
    const { ctx, page, errors } = await open(browser, base, { viewport: { width: 1280, height: 800 } });
    await shot(page, "web_00_loading");
    check(await frames(page) === 0, "emulator waits for the player to click Play");
    await page.click("#overlay");
    await wait(page, 2500);                       // boot + title fade-in
    const f = await frames(page);
    check(f > 100, `emulator runs (${f} frames in 2.5 s)`);
    const title = await pixels(page);
    check(colours(title) > 8, `title screen drawn (${colours(title)} colours)`);
    await shot(page, "web_01_title");

    await tapKey(page, "Enter"); await wait(page, 600);
    const intro = await pixels(page);
    check(diff(title, intro) > 1500, `Enter (START) opens the intro (${diff(title, intro)} px changed)`);
    await shot(page, "web_02_intro");
    await tapKey(page, "KeyX"); await wait(page, 400);
    const intro2 = await pixels(page);
    check(diff(intro, intro2) > 200, `X (A) turns the intro page (${diff(intro, intro2)} px)`);
    await tapKey(page, "KeyX"); await wait(page, 400);
    await tapKey(page, "KeyX"); await wait(page, 1200);
    const street = await pixels(page);
    await shot(page, "web_03_street");
    await tapKey(page, "KeyZ"); await wait(page, 300);  // B: dismiss the goal line
    const noBox = await pixels(page);
    check(diff(street, noBox) > 200, `Z (B) closes the goal box (${diff(street, noBox)} px)`);

    await page.keyboard.press("F6"); await wait(page, 300);
    const saved = await pixels(page);
    check(await page.evaluate(() => !!localStorage.getItem("paris-disparu-state-1")), "F6 saves a state in the browser");
    await page.keyboard.down("ArrowLeft"); await wait(page, 700); await page.keyboard.up("ArrowLeft");
    await wait(page, 200);
    const moved = await pixels(page);
    check(diff(saved, moved) > 30, `arrow key moves the player (${diff(saved, moved)} px)`);
    await shot(page, "web_04_walk");
    await page.keyboard.press("F9"); await wait(page, 100);
    await page.evaluate(() => new Promise(requestAnimationFrame));
    const loaded = await pixels(page);
    check(diff(saved, loaded) < diff(saved, moved), `F9 loads it back (${diff(saved, loaded)} px from the save)`);

    await page.keyboard.down("Enter"); await wait(page, 120); await page.keyboard.up("Enter"); await wait(page, 500);
    const notebook = await pixels(page);
    check(diff(loaded, notebook) > 1500, `Enter opens the notebook (${diff(loaded, notebook)} px)`);
    await shot(page, "web_05_notebook");
    check(errors.length === 0, "no errors in the console" + (errors.length ? ": " + errors.join(" | ") : ""));
    await ctx.close();
  }

  console.log("phone, touch buttons");
  {
    const { ctx, page, errors } = await open(browser, base, { ...devices["iPhone 13"] });
    await shot(page, "web_10_phone_loading");
    await page.tap("#overlay");
    await wait(page, 2500);
    const title = await pixels(page);
    check(await frames(page) > 100, "emulator runs after a tap");
    await page.tap('[data-btn="start"]'); await wait(page, 600);
    const intro = await pixels(page);
    check(diff(title, intro) > 1500, `on-screen START opens the intro (${diff(title, intro)} px)`);
    await page.tap('[data-btn="a"]'); await wait(page, 400);
    const intro2 = await pixels(page);
    check(diff(intro, intro2) > 200, `on-screen A turns the page (${diff(intro, intro2)} px)`);
    await page.tap('[data-btn="b"]'); await wait(page, 1200);   // B skips the intro
    await page.tap('[data-btn="b"]'); await wait(page, 300);    // and closes the goal line
    const street = await pixels(page);
    // Hold the left arm of the D-pad with a real touch point.
    const box = await page.locator("#dpad").boundingBox();
    const cdp = await ctx.newCDPSession(page);
    const pt = { x: box.x + box.width * 0.15, y: box.y + box.height / 2 };
    await cdp.send("Input.dispatchTouchEvent", { type: "touchStart", touchPoints: [pt] });
    await wait(page, 700);
    await cdp.send("Input.dispatchTouchEvent", { type: "touchEnd", touchPoints: [] });
    await wait(page, 200);
    const moved = await pixels(page);
    check(diff(street, moved) > 30, `holding the on-screen D-pad moves the player (${diff(street, moved)} px)`);
    await shot(page, "web_11_phone_walk");
    const noScroll = await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth);
    check(noScroll, "no horizontal scrolling on a phone");
    check(errors.length === 0, "no errors in the console" + (errors.length ? ": " + errors.join(" | ") : ""));
    await ctx.close();
  }

  console.log("in-game save kept in the browser; Episode 1 -> Episode 2");
  {
    const { ctx, page, errors } = await open(browser, base, { viewport: { width: 1280, height: 800 } });
    const two = await page.evaluate(() => !!window.GAME_ROM2_B64);
    if (two) check(await page.isVisible("#btn-ep") && (await page.textContent("#btn-ep")).includes("Episode 2"),
                   "an Episode 2 button");
    await page.click("#overlay");
    await wait(page, 2500);
    await tapKey(page, "Enter"); await wait(page, 600);    // title
    await tapKey(page, "KeyX"); await wait(page, 600);     // language: English
    await tapKey(page, "KeyZ"); await wait(page, 1500);    // skip the intro
    await tapKey(page, "KeyZ"); await wait(page, 300);
    await tapKey(page, "Backspace"); await wait(page, 800); // SELECT: sound off, saved
    const sram = await page.evaluate(() => atob(localStorage.getItem("paris-disparu-sram-1") || "").slice(0, 4));
    check(sram === "PDS4", `the game's own save is kept in localStorage (${JSON.stringify(sram)})`);
    if (two) {
      // The ROM writes "EP2!" at cartridge RAM 0x80 when the player picks "Go to
      // Episode 2" (checked by tests/play_test.py); put it there the same way.
      await page.evaluate(() => {
        const p = window.player, m = p.m, orig = m._emulator_was_ext_ram_updated;
        p.withExtRam((fd, buf) => { m._emulator_write_ext_ram(p.e, fd); buf.set([0x45, 0x50, 0x32, 0x21], 0x80); m._emulator_read_ext_ram(p.e, fd); });
        m._emulator_was_ext_ram_updated = () => { m._emulator_was_ext_ram_updated = orig; return 1; };
      });
      await wait(page, 1500);
      check(await page.textContent("header h1") === "PARIS DISPARU 2" &&
            await page.evaluate(() => window.player.saveKey) === "paris-disparu-2-state-1",
            "the mark loads Episode 2: " + await page.textContent("header h1"));
      check((await page.textContent("#btn-ep")).includes("Episode 1"), "the button now goes back to Episode 1");
      const ep1 = await page.evaluate(() => atob(localStorage.getItem("paris-disparu-sram-1")).charCodeAt(0x80));
      check(ep1 === 0, "the mark is cleared in Episode 1's save");
      check(await frames(page) > 0, "Episode 2 runs");
      await shot(page, "web_20_episode2");
      await page.reload();
      await page.waitForFunction(() => !document.querySelector("#overlay button").disabled);
      check(await page.textContent("header h1") === "PARIS DISPARU 2", "a reload stays on Episode 2");
      await page.click("#btn-ep");
      check(await page.textContent("header h1") === "PARIS DISPARU", "the button switches back to Episode 1");
      await page.click("#overlay");
      await wait(page, 500);
      const back = await page.evaluate(() => window.player.withExtRam((fd, buf) => {
        window.player.m._emulator_write_ext_ram(window.player.e, fd);
        return String.fromCharCode(...buf.slice(0, 4));
      }));
      check(back === "PDS4", `Episode 1's save is back in the cartridge after a reload (${JSON.stringify(back)})`);
    }
    check(errors.length === 0, "no errors in the console" + (errors.length ? ": " + errors.join(" | ") : ""));
    await ctx.close();
  }

  console.log("opened straight from the unzipped folder (file://)");
  {
    const { ctx, page, errors } = await open(browser, "file://" + path.join(DIR, "index.html"));
    await page.click("#overlay");
    await wait(page, 1500);
    check(await frames(page) > 60, "runs without a web server");
    check(errors.length === 0, "no errors in the console" + (errors.length ? ": " + errors.join(" | ") : ""));
    await ctx.close();
  }

  await browser.close();
  srv.close();
  console.log(failed ? `${failed} check(s) FAILED` : "all web checks passed");
  process.exit(failed ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(1); });
