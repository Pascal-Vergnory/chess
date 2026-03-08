## Evaluating the effects of a change in the chess engine (engine.c)

To "evaluate" if a change in the chess engines improves it or not, I have developped tester.exe (and chesst.exe to go with it).
Tester.exe will allow to run multiple parties between two chess versions to see which one wins more than the other.
(works only on Windows for the moment...)

Two new source files are added:
- tester.c (the tester)
- chesst.c (to allow the chess engine to talk with the tester)

To build tester.exe and chesst.exe, use `build_tester.bat`

Example of use:
- run `build_tester.bat` before the change in engine.c
- rename chesst.exe into chesst_v0.exe
- make the changes in engine.c
- run again `build_tester.bat`
- launch tester.exe to compare chesst_org.exe and chesst.exe by typing:

./tester.exe chesst_v0.exe chesst.exe

## Remarks on the test results

Unfortunately the outcomes are very random and the relation "wins on average over" is not transitive. The results may be mis-leading.

For example `chesst_v2` may beat `chesst_v1` on average and `chesst_v3` may beat `chesst_v2`, but `chesst_v1` may actually beat `chesst_v3`
