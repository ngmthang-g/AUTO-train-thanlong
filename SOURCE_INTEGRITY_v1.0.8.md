# NewCore v1.0.8 source integrity

The GitHub connector previously corrupted the two large C++ translation units during transport. v1.0.8 therefore stores those translation units as ordered `.inc` fragments in the same source directory and compiles them through a tiny wrapper `.cpp`.

This is a transport/storage workaround only. The concatenated bridge fragments equal the intended v1.0.8 `bridge.cpp`; the concatenated controller fragments equal the intended v1.0.8 `main.cpp`.

Golden-rule audit scans `.cpp`, `.h`, and `.inc` files. Gameplay actions remain locked.

Intended full-source Git blob identities before splitting:
- bridge.cpp: 8482a2890511bd7f9e6e672c1a08e32f53df5d1c (37931 bytes)
- controller/main.cpp: 964f2412a49208d051682730fe0277b817ad88b2 (34614 bytes)

Fragment Git blob identities:
Bridge:
- 01 5cc8e6551f0d24ce7f059f111ca6bd83aff3aa79
- 02 797242941c6b503e0a99d2be3efb503a8b57946c
- 03 70587d952f760c5872812975b5863ea324bf5bdb
- 04 62569faeb8ed6bbf3b410397bd44e8f88192740d
- 05 2886d800201d5c335a0ad9cac9f6f1982f7d8416
- 06 8e215b3f5195a742e93a44c4cb4833292ca0bb75
- 07 a6b42a8267cbcabe589bc15b6496459f3a65e50e
- 08 83e1484bd61d43cf3aaa76e837781fbc6b15cd26
Controller:
- 01 fee605ed56e4cd90d3efc210c2de3ac7d2c5b3b2
- 02 334c259466c16c26adf0bf914cdd99707c93bf1e
- 03 80de9596f7334ab652bf23b2f7f29ba8e60965e0
- 04 b91036696ce1a08731772a79c7cc5bd180689df1
- 05 e5a9ea6a370bdb8062b5db6facb2c019d5906c36
- 06 2e64ea32442539dd98267641a406767041d84bd5
- 07 4c327b1a4a9e0d2026a035eed4479e1e8d2b95c3
- 08 e634fa8b679d217780aa9258d4fb08b51848ff74
