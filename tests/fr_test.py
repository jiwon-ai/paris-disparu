"""French option smoke test (PyBoy): the language menu, French intro, caption,
notebook, a bark, a dialogue with its French choice. Reuses the helpers of
play_test.py (everything above its "boot" section).

Usage: python3 tests/fr_test.py [rom] [screenshot dir]
"""
import os
import sys

_here = os.path.dirname(os.path.abspath(__file__))
_src = open(os.path.join(_here, "play_test.py"), encoding="utf-8").read().split('print("boot")')[0]
__file__ = os.path.join(_here, "play_test.py")
exec(compile(_src, "play_test.py", "exec"))

tick(150)
tap("start", 20)
check("Français" in window_row(10), "the language menu offers French: %r" % window_row(10))
tap("down", 8)
tap("a", 20)
check(mem[sym["_lang_fr"]] == 1 and mem[sym["_lang"]] == 0, "French chosen (lang_fr=1, Korean engine off)")
check("PARIS 21 NOV 1967" in window_row(3), "French intro page 1: %r" % window_row(3))
shot("fr_00_intro")
tap("a", 10)
check("CETTE NUIT" in window_row(3), "French intro page 2: %r" % window_row(3))
tap("a", 10)
check("COMMANDES" in window_row(3), "French intro page 3: %r" % window_row(3))
shot("fr_00_controls")
tap("a", 50)
check(window_on() and "Retrouve Fernand," in window_row(1), "French opening caption: %r" % window_row(1))
shot("fr_02_street")
tap("b", 4)
tap("start", 10)
check("CARNET" in window_row(1) and "Missions" in window_row(5), "French notebook: %r %r" % (window_row(1), window_row(5)))
shot("fr_04_notebook")
tap("start", 10)
street_to_baltard()
to_dede()
bark = page_text()
check(mem[0xFE00 + 35 * 4] != 0, "Dédé calls out (speech bubble)")
tap("a", 70)
first = page_text()
check(first.startswith("DÉDÉ : Toi !"), "French dialogue: %r" % first)
for _ in range(12):
    if "▶" in window_row(2):
        break
    tap("a", 60)
check("Oui" in window_row(2) and "Non" in window_row(2), "French choice: %r" % window_row(2))
shot("fr_05_choice")
tap("a", 60)
check(page_text().startswith("DÉDÉ : Bien."), "the answer is in French: %r" % page_text())

print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
